#include "elf.h"

#include "printf.h"
#include "utils.h"

void print_elf_header(const struct elf_header* ehdr) {
  printf("ELF header:\r\n");

  // Print magic number
  printf("\tMagic:\t\t\t");
  for (int i = 0; i < EI_NIDENT; ++i) {
    printf("%x ", ehdr->e_ident[i]);
  }
  printf("\r\n");

  // Print class
  printf("\tClass:\t\t\t");
  switch (ehdr->e_ident[EI_CLASS]) {
    case ELFCLASSNONE: {
      printf("Invalid");
      break;
    }
    case ELFCLASS32: {
      printf("ELF32");
      break;
    }
    case ELFCLASS64: {
      printf("ELF64");
      break;
    }
    default: {
      printf("Unsupported");
    }
  }
  printf("\r\n");

  // Print Data
  printf("\tData:\t\t\t");
  switch (ehdr->e_ident[EI_DATA]) {
    case ELFDATANONE: {
      printf("Invalid");
      break;
    }
    case ELFDATA2LSB: {
      printf("Two's complement, little endian");
      break;
    }
    case ELFDATA2MSB: {
      printf("Two's complement, big endian");
      break;
    }
    default: {
      printf("Unsupported");
    }
  }
  printf("\r\n");

  // Print version
  printf("\tVersion:\t\t");
  switch (ehdr->e_ident[EI_VERSION]) {
    case EV_NONE: {
      printf("%c (invalid)", '0' + EV_NONE);
      break;
    }
    case EV_CURRENT: {
      printf("%c (current)", '0' + EV_CURRENT);
      break;
    }
    default: {
      printf("Unsupported");
    }
  }
  printf("\r\n");

  // Print type
  printf("\tType:\t\t\t");
  switch (ehdr->e_type) {
    case ET_NONE: {
      printf("No file type");
      break;
    }
    case ET_REL: {
      printf("REL (Relocatable file)");
      break;
    }
    case ET_EXEC: {
      printf("EXEC (Executable file)");
      break;
    }
    case ET_DYN: {
      printf("DYN (Shared object file)");
      break;
    }
    case ET_CORE: {
      printf("CORE (Core file)");
      break;
    }
    case ET_LOPROC: {
      printf("LOPROC (Processor-specific)");
      break;
    }
    case ET_HIPROC: {
      printf("HIPROC (Processor-specific)");
      break;
    }
    default: {
      printf("Unsupported");
    }
  }
  printf("\r\n");

  // Print machine
  printf("\tMachine:\t\t");
  switch (ehdr->e_machine) {
    case ET_NONE: {
      printf("No machine");
      break;
    }
    case EM_M32: {
      printf("AT&T WE 32100");
      break;
    }
    case EM_SPARC: {
      printf("SPARC");
      break;
    }
    case EM_386: {
      printf("Intel architecture");
      break;
    }
    case EM_68K: {
      printf("Motorola 68000");
      break;
    }
    case EM_88K: {
      printf("Motorola 88000");
      break;
    }
    case EM_860: {
      printf("Intel 80860");
      break;
    }
    case EM_MIPS: {
      printf("MIPS RS3000 Big-Endian");
      break;
    }
    case EM_MIPS_RS4_BE: {
      printf("MIPS RS4000 Big-Endian");
      break;
    }
    case EM_AARCH64: {
      printf("AArch64");
      break;
    }
    default: {
      printf("Reserved for future use");
    }
  }
  printf("\r\n");

  // Print entry
  printf("\tEntry point address:\t%p\r\n", (uint64)ehdr->e_entry);

  // Print program headers
  printf("\nProgram headers:\r\n");
  for (int i = 0, offset = ehdr->e_phoff; i < ehdr->e_phnum;
       ++i, offset += sizeof(struct elf_prog_header)) {
    print_elf_prog_header((struct elf_prog_header*)((char*)ehdr + offset));
  }
}

void print_elf_prog_header(const struct elf_prog_header* ephdr) {
  // Print segment type
  switch (ephdr->p_type) {
    case PT_NULL: {
      printf("\t- Unused segment");
      break;
    }
    case PT_LOAD: {
      printf("\t- Loadable segment");
      break;
    }
    case PT_DYNAMIC: {
      printf("\t- Dynamic linking information segment");
      break;
    }
    case PT_INTERP: {
      printf("\t- Path name to an interpreter");
      break;
    }
    case PT_NOTE: {
      printf("\t- Auxiliary information segment");
      break;
    }
    case PT_SHLIB: {
      printf("\t- Unspecified segment");
      break;
    }
    case PT_PHDR: {
      printf("\t- Program header segment");
      break;
    }
    case PT_LOPROC:
    case PT_HIPROC: {
      printf("\t- Processor-specific segment");
      break;
    }
    default: {
      printf("\t- Unsupported segment type");
    }
  }

  printf(": ");

  // Print offset + size
  printf("%x -- %x (%d)\r\n", ephdr->p_offset,
         ephdr->p_offset + ephdr->p_filesz, ephdr->p_filesz);
}

// Apply relocations present in the .dynamic section. Each relocation
// refers to an address of a global variable or function in the
// program.  For each address, we add the process' base address
// (pa_start), so it points to the correct address after loading
void apply_relocations(void* pa_start, struct elf_header* ehdr) {
  for (int i = 0, offset = ehdr->e_phoff; i < ehdr->e_phnum;
       ++i, offset += sizeof(struct elf_prog_header)) {
    struct elf_prog_header* ph =
        (struct elf_prog_header*)((char*)ehdr + offset);

    if (ph->p_type != PT_DYNAMIC) {
      continue;
    }

    // Walk the dynamic section to find the relocation table.
    uint64 rela_vaddr = 0;
    uint64 rela_size = 0;
    for (struct elf_dyn* dyn = (struct elf_dyn*)((char*)pa_start + ph->p_vaddr);
         dyn->d_tag != DT_NULL; dyn++) {
      if (dyn->d_tag == DT_RELA) {
        rela_vaddr = dyn->d_val;
      } else if (dyn->d_tag == DT_RELASZ) {
        rela_size = dyn->d_val;
      }
    }

    if (rela_vaddr == 0 || rela_size == 0) break;

    struct elf_rela* rela = (struct elf_rela*)((char*)pa_start + rela_vaddr);
    int nrela = rela_size / sizeof(struct elf_rela);

    for (int r = 0; r < nrela; r++) {
      uint32 type = ELF_R_TYPE(rela[r].r_info);
      if (type != R_AARCH64_RELATIVE) {
        panic("Unsupported relocation type: %d", type);
      }

      // Modify the address given by this relocation
      uint64* patch = (uint64*)((char*)pa_start + rela[r].r_offset);
      *patch = (uint64)pa_start + rela[r].r_addend;
    }

    return;  // Stop processing after .dynamic section is found (there is only
             // one)
  }
}

void* load_elf(struct elf_header* ehdr, void* pa_start) {
  // Parse ELF header
  if (ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
      ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3) {
    panic("Invalid ELF magic: %x %x %x %x", ehdr->e_ident[EI_MAG0],
          ehdr->e_ident[EI_MAG1], ehdr->e_ident[EI_MAG2],
          ehdr->e_ident[EI_MAG3]);
  }

  // Sanity checks
  if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
    panic("Not a ELF64");
  }
  if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
    panic("Unsupported endianess");
  }
  if (ehdr->e_machine != EM_AARCH64) {
    panic("Machine is not AArch64");
  };

  // Load program headers in memory
  bool found = false;
  struct elf_prog_header* ph;

  for (int i = 0, offset = ehdr->e_phoff; i < ehdr->e_phnum;
       ++i, offset += sizeof(struct elf_prog_header)) {
    ph = (struct elf_prog_header*)((char*)ehdr + offset);

    // Only load loadable segments
    if (ph->p_type != PT_LOAD) {
      continue;
    }

    // Sanity checks
    if (ph->p_memsz < ph->p_filesz)
      panic("In program header: memsz < filesz\r\n");
    if (ph->p_vaddr + ph->p_memsz < ph->p_vaddr)
      panic("In program header: load would overflow");

    found = true;

    void* src = (char*)ehdr + ph->p_offset;
    void* dest = pa_start + ph->p_vaddr;

    memmove(dest, src, ph->p_filesz);

    // Zero the BSS region, which is for data expected to be initialized to zero
    // This is the portion of the segment that exists in memory
    // (p_memsz) but not in the file (p_filesz).
    if (ph->p_memsz > ph->p_filesz) {
      memset((char*)dest + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
    }
  }

  if (!found) {
    panic("Could not find any PT_LOAD segment in ELF");
  }

  // Apply any relocations present in the ELF file.
  // Relocations are used to adjust any addresses of global variables or
  // functions so that they reflect the correct addresses of where the program
  // is loaded into memory.
  apply_relocations(pa_start, ehdr);

  // return the entry point of the loaded ELF, adjusted by the physical address
  // at which the process starts.
  return (void*)((char*)ehdr->e_entry + (uint64)pa_start);
}
