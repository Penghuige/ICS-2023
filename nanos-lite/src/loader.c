#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);

extern int fs_open(const char *pathname, int flags, int mode);
extern int fs_close(int fd);
extern size_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_write(int fd, void *buf, size_t len);
extern size_t fs_lseek(int fd, size_t offset, int whence);

 uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  if (fd < 0) {
    Log("File not found: %s", filename);
    return 0;
  }
  assert(fd >= 2);

  // make sure the elf read is the fs_read
  Elf_Ehdr ehdr;

  assert(fs_read(fd, &ehdr, sizeof(ehdr)) == sizeof(ehdr));
  // display all value in Elf_Ehdr
  printf("ehdr.e_ident: %x\n", *(uint32_t *)ehdr.e_ident);
  printf("ehdr.e_type: %x\n", ehdr.e_type);
  printf("ehdr.e_machine: %x\n", ehdr.e_machine);
  printf("ehdr.e_version: %x\n", ehdr.e_version);
  printf("ehdr.e_entry: %x\n", ehdr.e_entry);
  printf("ehdr.e_phoff: %x\n", ehdr.e_phoff);
  printf("ehdr.e_shoff: %x\n", ehdr.e_shoff);
  printf("ehdr.e_flags: %x\n", ehdr.e_flags);
  printf("ehdr.e_ehsize: %x\n", ehdr.e_ehsize);
  printf("ehdr.e_phentsize: %x\n", ehdr.e_phentsize);
  printf("ehdr.e_phnum: %x\n", ehdr.e_phnum);
  printf("ehdr.e_shentsize: %x\n", ehdr.e_shentsize);
  printf("ehdr.e_shnum: %x\n", ehdr.e_shnum);
  printf("ehdr.e_shstrndx: %x\n", ehdr.e_shstrndx);

  // replace the ramdisk_read with fs_read, it used to be zero while ramdisk_read.
  //ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  Log("[loader] file %d ehdr.e_ident: %x\n", fd, *(uint32_t *)ehdr.e_ident);

  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);

  //printf("ehdr.e_phnum: %d\n", ehdr.e_phnum);

  Elf_Phdr phdr[ehdr.e_phnum];

  assert(fs_lseek(fd, ehdr.e_ehsize, SEEK_SET) == ehdr.e_ehsize);
  //assert(0);
  // get all the phdr
  assert(fs_read(fd, phdr, ehdr.e_phnum * sizeof(Elf_Phdr)) == ehdr.e_phnum * sizeof(Elf_Phdr));

  // ramdisk_read(phdr, ehdr.e_ehsize, ehdr.e_phnum * sizeof(Elf_Phdr));

  for(int i = 0 ; i < ehdr.e_phnum ; i++){
    if(phdr[i].p_type == PT_LOAD){
      // ramdisk_read((void *)phdr[i].p_vaddr, phdr[i].p_offset, phdr[i].p_memsz);
      // need to set offset
      assert(fs_lseek(fd, phdr[i].p_offset, SEEK_SET) == phdr[i].p_offset);
      assert(fs_read(fd, (void *)phdr[i].p_vaddr, phdr[i].p_filesz) == phdr[i].p_filesz);
      // set zero
      memset((void *)(phdr[i].p_vaddr + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
    }
  }

  assert(fs_close(fd) == 0);
  
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  if(filename == NULL)
  {
    printf("filename is NULL\n");
  }
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

