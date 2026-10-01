#!/usr/bin/env python3
"""Generate a GDB script that loads debug symbols for all user-space ELF
programs embedded in the kernel image.

Usage:
    python3 gen_user_symbols.py [-o output.gdb] <kernel_elf> <user_elf_dir>

The script reads the global `executables` array from the kernel ELF to
determine which user programs are loaded and in what order.  The array
index gives the PID, which determines the runtime base address
(PROC_START + pid * PROC_SIZE).  For each program, it emits an
add-symbol-file GDB command with the correct relocated address for
every ALLOC section.

The output can be sourced from .gdbinit to get proper debug symbols
(breakpoints, variable inspection, etc.) for user-space code.
"""

import argparse
import os
import re
import struct
import subprocess
import sys

# Defaults, overridden by parsing kernel/limits.h if available.
PROC_START = 0xA0000
PROC_SIZE = 0x10000


def parse_limits(limits_path):
    """Read PROC_START and PROC_SIZE from kernel/limits.h."""
    global PROC_START, PROC_SIZE
    try:
        with open(limits_path) as f:
            for line in f:
                m = re.match(r"#define\s+PROC_START\s+(0x[0-9a-fA-F]+|\d+)", line)
                if m:
                    PROC_START = int(m.group(1), 0)
                m = re.match(r"#define\s+PROC_SIZE\s+(0x[0-9a-fA-F]+|\d+)", line)
                if m:
                    PROC_SIZE = int(m.group(1), 0)
    except FileNotFoundError:
        pass


def get_nm_symbols(kernel_elf):
    """Run nm -S on the kernel ELF.  Return:
      - blob_syms: {address: program_name} for _binary_user_*_elf_start symbols
      - exec_addr: address of the `executables` symbol (or None)
      - exec_size: size of the `executables` symbol in bytes (or None)
    """
    result = subprocess.run(
        [f"{TOOLPREFIX}nm", "-S", kernel_elf], capture_output=True, text=True, check=True
    )
    blob_syms = {}
    exec_addr = None
    exec_size = None

    for line in result.stdout.splitlines():
        parts = line.split()
        if len(parts) < 3:
            continue

        # nm -S output: "addr size type name" (4 cols) or "addr type name" (3 cols)
        if len(parts) >= 4 and len(parts[1]) > 1 and all(c in "0123456789abcdef" for c in parts[1]):
            addr, size, typ, name = int(parts[0], 16), int(parts[1], 16), parts[2], parts[3]
        else:
            addr, typ, name = int(parts[0], 16), parts[1], parts[2]
            size = None

        if re.fullmatch(r"_binary_user_[A-Za-z0-9_]+_elf_start", name):
            # Extract program name: _binary_user_<name>_elf_start
            prog = name.removeprefix("_binary_user_").removesuffix("_elf_start")
            blob_syms[addr] = prog

        if name == "executables":
            exec_addr = addr
            exec_size = size

    return blob_syms, exec_addr, exec_size


def get_load_segments(kernel_elf):
    """Return a list of (vaddr, file_offset, filesz) for LOAD segments."""
    result = subprocess.run(
        [f"{TOOLPREFIX}readelf", "-lW", kernel_elf], capture_output=True, text=True, check=True
    )
    segments = []
    for line in result.stdout.splitlines():
        # Match LOAD lines:  LOAD  0x010000 0x0000000000080000 ... 0x01af00 ...
        m = re.match(
            r"\s*LOAD\s+(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)",
            line,
        )
        if m:
            file_offset = int(m.group(1), 16)
            vaddr = int(m.group(2), 16)
            # paddr = m.group(3)
            filesz = int(m.group(4), 16)
            segments.append((vaddr, file_offset, filesz))
    return segments


def vaddr_to_file_offset(vaddr, segments):
    """Convert a virtual address to a file offset using LOAD segment mappings."""
    for seg_vaddr, seg_offset, seg_filesz in segments:
        if seg_vaddr <= vaddr < seg_vaddr + seg_filesz:
            return seg_offset + (vaddr - seg_vaddr)
    return None


def read_executables_array(kernel_elf, exec_addr, exec_size, segments):
    """Read the executables array from the kernel ELF file.
    Returns a list of pointer values (addresses of ELF blobs)."""
    file_offset = vaddr_to_file_offset(exec_addr, segments)
    if file_offset is None:
        print(
            f"error: could not map executables address 0x{exec_addr:x} to file offset",
            file=sys.stderr,
        )
        sys.exit(1)

    num_entries = exec_size // 8
    with open(kernel_elf, "rb") as f:
        f.seek(file_offset)
        data = f.read(exec_size)

    return list(struct.unpack(f"<{num_entries}Q", data))


def elf_name_to_symbol(name):
    """Convert an ELF filename (e.g. 'loader-test') to the objcopy-mangled
    program name.  objcopy replaces any non-alphanumeric character with '_'."""
    return re.sub(r"[^A-Za-z0-9]", "_", name)


def get_alloc_sections(elf_path):
    """Return a list of (name, vaddr) for every ALLOC section with
    nonzero size in the given ELF file."""
    result = subprocess.run(
        [f"{TOOLPREFIX}readelf", "-SW", elf_path], capture_output=True, text=True, check=True
    )
    sections = []
    for line in result.stdout.splitlines():
        # Section header lines look like:
        #   [ 1] .text  PROGBITS  0000000000000000  000158  0010f4  ...  AX  ...
        m = re.match(
            r"\s*\[\s*\d+\]\s+(\S+)\s+\S+\s+([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+([0-9a-fA-F]+)",
            line,
        )
        if not m:
            continue
        name = m.group(1)
        vaddr = int(m.group(2), 16)
        size = int(m.group(3), 16)

        if size == 0:
            continue
        # Check for the A(lloc) flag in the flags column.
        flag_part = line[m.end():]
        flag_match = re.search(r"\b([WAXMSILTCGE]+)\b", flag_part)
        if not flag_match or "A" not in flag_match.group(1):
            continue

        sections.append((name, vaddr))
    return sections


def find_user_elf(user_elf_dir, mangled_name):
    """Find the .elf file in user_elf_dir whose mangled name matches.
    Returns the path, or None."""
    for f in os.listdir(user_elf_dir):
        if not f.endswith(".elf"):
            continue
        prog_name = f.removesuffix(".elf")
        if elf_name_to_symbol(prog_name) == mangled_name:
            return os.path.join(user_elf_dir, f)
    return None


def generate_gdb_commands(kernel_elf, user_elf_dir):
    """Generate GDB add-symbol-file commands for all user ELFs."""
    # Try to read PROC_START/PROC_SIZE from limits.h relative to kernel ELF
    kernel_dir = os.path.dirname(kernel_elf)
    limits_path = os.path.join(kernel_dir, "limits.h")
    parse_limits(limits_path)

    blob_syms, exec_addr, exec_size = get_nm_symbols(kernel_elf)

    if exec_addr is None or exec_size is None:
        print(
            "warning: 'executables' symbol not found in kernel ELF. "
            "Make sure the executables array is a global variable.\n"
            "GDB user-space symbol loading will not be available.",
            file=sys.stderr,
        )
        return []

    if not blob_syms:
        print(
            f"warning: no _binary_user_*_elf_start symbols found in {kernel_elf}",
            file=sys.stderr,
        )
        return []

    segments = get_load_segments(kernel_elf)
    pointers = read_executables_array(kernel_elf, exec_addr, exec_size, segments)

    lines = ["# Generated by gen_user_symbols.py — do not edit"]
    lines.append(f"# Kernel: {kernel_elf}")
    lines.append(f"# PROC_START=0x{PROC_START:x}  PROC_SIZE=0x{PROC_SIZE:x}")
    lines.append("")

    for pid, ptr in enumerate(pointers):
        if ptr not in blob_syms:
            print(
                f"warning: executables[{pid}] = 0x{ptr:x} does not match "
                f"any _binary_user_*_elf_start symbol",
                file=sys.stderr,
            )
            continue

        mangled_name = blob_syms[ptr]
        elf_path = find_user_elf(user_elf_dir, mangled_name)
        if elf_path is None:
            print(
                f"warning: no .elf file found for program '{mangled_name}' "
                f"in {user_elf_dir}",
                file=sys.stderr,
            )
            continue

        base_addr = PROC_START + pid * PROC_SIZE
        sections = get_alloc_sections(elf_path)

        if not sections:
            print(f"warning: no ALLOC sections found in {elf_path}", file=sys.stderr)
            continue

        # add-symbol-file requires the .text address as the positional arg.
        text_addr = None
        other_sections = []
        for name, vaddr in sections:
            if name == ".text":
                text_addr = base_addr + vaddr
            else:
                other_sections.append((name, base_addr + vaddr))

        if text_addr is None:
            print(f"warning: no .text section found in {elf_path}", file=sys.stderr)
            continue

        # Build the command with line continuations for readability.
        parts = [f"add-symbol-file {elf_path} 0x{text_addr:x}"]
        for name, addr in other_sections:
            parts.append(f"  -s {name} 0x{addr:x}")

        lines.append(f"# executables[{pid}]: {mangled_name} -> base 0x{base_addr:x}")
        lines.append(" \\\n".join(parts))
        lines.append("")

    return lines


def main():
    parser = argparse.ArgumentParser(
        description="Generate GDB script to load user-space debug symbols."
    )
    parser.add_argument("kernel_elf", help="Path to the kernel ELF file")
    parser.add_argument("user_elf_dir", help="Directory containing user .elf files")
    parser.add_argument("-t", "--toolprefix", default=None, help="Toolchain prefix (e.g., aarch64-elf-)")
    parser.add_argument(
        "-o", "--output", default=None, help="Output file (default: stdout)"
    )
    args = parser.parse_args()

    if args.toolprefix:
        global TOOLPREFIX
        TOOLPREFIX = args.toolprefix

    lines = generate_gdb_commands(args.kernel_elf, args.user_elf_dir)
    output = "\n".join(lines) + "\n" if lines else ""

    if args.output:
        with open(args.output, "w") as f:
            f.write(output)
    else:
        sys.stdout.write(output)

TOOLPREFIX = ""  # Global variable to hold the toolchain prefix

if __name__ == "__main__":
    main()
