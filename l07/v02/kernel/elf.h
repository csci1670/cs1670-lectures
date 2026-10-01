#ifndef __ELF_H
#define __ELF_H

#include "types.h"

#define ELF_MAGIC 0x464C457FU  // '\x7F'.'E'.'L'.'F' (little endian)

// E_IDENT related
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

enum eident {
  EI_MAG0 = 0,
  EI_MAG1 = 1,
  EI_MAG2 = 2,
  EI_MAG3 = 3,
  EI_CLASS = 4,    // file format - 0: invalid, 1: 32 bit, 2: 64 bit
  EI_DATA = 5,     // encoding - 0: inv, 1: little endian, 2: big endian
  EI_VERSION = 6,  // header version
  EI_PAD = 7,      // padding - reserved for future use
  EI_NIDENT = 16   // size
};

enum ei_class { ELFCLASSNONE = 0, ELFCLASS32 = 1, ELFCLASS64 = 2 };

enum ei_data { ELFDATANONE = 0, ELFDATA2LSB = 1, ELFDATA2MSB = 2 };

enum ei_version { EV_NONE = 0, EV_CURRENT = 1 };

enum e_type {
  ET_NONE = 0,
  ET_REL = 1,
  ET_EXEC = 2,
  ET_DYN = 3,
  ET_CORE = 4,
  ET_LOPROC = 0xff00,
  ET_HIPROC = 0xffff
};

enum e_machine {
  EM_M32 = 1,
  EM_SPARC = 2,
  EM_386 = 3,
  EM_68K = 4,
  EM_88K = 5,
  EM_860 = 7,
  EM_MIPS = 8,
  EM_MIPS_RS4_BE = 10,
  RESERVED,
  EM_AARCH64 = 0xb7
};

struct elf_header {
  uint8 e_ident[EI_NIDENT];  // identification number (see above)
  uint16
      e_type;  // object file (none, relocatable, executable, shared, core...)
  uint16 e_machine;    // architecture (none, AT&T, SPARC, Intel, Motorola...)
  uint32 e_version;    // object file version (0: invalid, 1: current)
  uint64 e_entry;      // entry point (usually address of the _start symbol)
  uint64 e_phoff;      // program header table's offset (in bytes)
  uint64 e_shoff;      // section header table's offset (in bytes)
  uint32 e_flags;      // processor-specific flags associated with the file
  uint16 e_ehsize;     // ELF header's size (in bytes)
  uint16 e_phentsize;  // Program header table entry size (in bytes)
  uint16 e_phnum;      // Number of entries in the program header table
  uint16 e_shentsize;  // Section header size (in bytes)
  uint16 e_shnum;      // Number of entries in the section header table
  uint16 e_shstrndx;   // Section header tbl id for e in section name string tbl
};

enum p_type {
  PT_NULL = 0,
  PT_LOAD = 1,
  PT_DYNAMIC = 2,
  PT_INTERP = 3,
  PT_NOTE = 4,
  PT_SHLIB = 5,
  PT_PHDR = 6,
  PT_LOPROC = 0x70000000,
  PT_HIPROC = 0x7fffffff
};

struct elf_prog_header {
  uint32 p_type;    // Segment described
  uint32 p_flags;   // Segment-dependant flags (e.g.: R/W/E permissions)
  uint64 p_offset;  // Offset of this segment from the beginning of the file
  uint64 p_vaddr;   // VA at which the first byte of this seg is in memory
  uint64 p_paddr;   // Segment PA (not always relevant)
  uint64 p_filesz;  // Size (in bytes) of this segment in the file image
  uint64 p_memsz;   // Size (in bytes) of this segment in memory
  uint64 p_align;
};

// Dynamic section entry (found via PT_DYNAMIC program header)
struct elf_dyn {
  int64 d_tag;   // Entry type (DT_RELA, DT_RELASZ, etc.)
  uint64 d_val;  // Integer or address value
};

// Dynamic section tags
#define DT_NULL 0     // End of dynamic section
#define DT_RELA 7     // Address of relocation table (with addends)
#define DT_RELASZ 8   // Total size of the RELA relocation table (bytes)
#define DT_RELAENT 9  // Size of each RELA relocation entry (bytes)

// Relocation entry with addend (Elf64_Rela)
struct elf_rela {
  uint64 r_offset;  // Location to apply the relocation (vaddr)
  uint64 r_info;    // Relocation type (low 32 bits) and symbol index (high 32)
  int64 r_addend;   // Constant addend
};

// Extract the relocation type from r_info
#define ELF_R_TYPE(info) ((uint32)((info) & 0xFFFFFFFF))

// AArch64 relocation types
#define R_AARCH64_RELATIVE 0x403  // Base-relative: *(offset) = base + addend

// Helper functions to print the parsed ELF header and program headers (for
// debugging)
void print_elf_header(const struct elf_header*);
void print_elf_prog_header(const struct elf_prog_header*);

// Load an ELF executable (currently stored in memory at ehdr) into process
// memory starting at the given physical address (pa_start).
void* load_elf(struct elf_header* ehdr, void* pa_start);

#endif  // __ELF_H
