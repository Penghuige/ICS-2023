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

static uintptr_t loader(PCB *pcb, const char *filename)
{
  Elf_Ehdr ehdr;

  // ramdisk_read(&ehdr, 0, sizeof(ehdr));
  int fd = fs_open(filename, 0, 0);
  assert(fd != -1);

  fs_read(fd, &ehdr, sizeof(ehdr));

  char riscv32_magic_num[] = {0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // printf("magic number is %s\n", (char *)(ehdr.e_ident));
  assert(strcmp((char *)(ehdr.e_ident), riscv32_magic_num) == 0);

  uint32_t entry = ehdr.e_entry;
  uint32_t ph_offset = ehdr.e_phoff;
  uint32_t ph_num = ehdr.e_phnum;

  Elf_Phdr phdr;
  for (int i = 0; i < ph_num; ++i)
  {
    // ramdisk_read(&phdr, ph_offset + i * sizeof(phdr), sizeof(phdr));
    fs_lseek(fd, ph_offset + i * sizeof(phdr), SEEK_SET);
    fs_read(fd, &phdr, sizeof(phdr));
    if (phdr.p_type != PT_LOAD)
      continue;

    // printf("load program header %d", i);

    uint32_t offset = phdr.p_offset;
    uint32_t file_size = phdr.p_filesz;
    uint32_t p_vaddr = phdr.p_vaddr;
    uint32_t mem_size = phdr.p_memsz;

    printf("load program from [%p, %p] to [%p, %p]\n", offset, file_size, p_vaddr, mem_size);
#ifdef USR_SPACE_ENABLE
    int left_size = file_size;
    fs_lseek(fd, offset, SEEK_SET);
    // printf("vaddr is %p\n", p_vaddr);
    if (!ISALIGN(p_vaddr))
    {
      void *pg_p = new_page(1);
      int read_len = min(PGSIZE - OFFSET(p_vaddr), left_size);
      left_size -= read_len;
      assert(fs_read(fd, pg_p + OFFSET(p_vaddr), read_len) >= 0);
      map(&pcb->as, (void *)p_vaddr, pg_p, PTE_R | PTE_W | PTE_X);
      p_vaddr += read_len;
    }

    for (; p_vaddr < phdr.p_vaddr + file_size; p_vaddr += PGSIZE)
    {
      assert(ISALIGN(p_vaddr));
      void *pg_p = new_page(1);
      memset(pg_p, 0, PGSIZE);
      // int len = min(PGSIZE, file_size - fs_lseek(fd, 0, SEEK_CUR));
      int read_len = min(PGSIZE, left_size);
      left_size -= read_len;
      assert(fs_read(fd, pg_p, read_len) >= 0);
      map(&pcb->as, (void *)p_vaddr, pg_p, PTE_R | PTE_W | PTE_X);
    }
    // printf("p_vaddr is %p\n", (void *)p_vaddr);
    p_vaddr = NEXT_PAGE(p_vaddr);
    printf("p_vaddr is %p next page, end of uninitialized space is %p\n", (void *)p_vaddr, (void *)(phdr.p_vaddr + mem_size));
    for (; p_vaddr < phdr.p_vaddr + mem_size; p_vaddr += PGSIZE)
    {
      assert(ISALIGN(p_vaddr));
      void *pg_p = new_page(1);
      memset(pg_p, 0, PGSIZE);
      map(&pcb->as, (void *)p_vaddr, pg_p, PTE_R | PTE_W | PTE_X);
    }
#else
    // ramdisk_read((void *)vaddr, offset, file_size);
    fs_lseek(fd, offset, SEEK_SET);
    fs_read(fd, (void *)p_vaddr, file_size);
    memset((void *)(p_vaddr + file_size), 0, mem_size - file_size);
#endif
    assert(mem_size >= file_size);
  }

  // printf("max brk is at %p when load\n", pcb->max_brk);
  assert(fs_close(fd) != -1);

  return entry;
}
 uintptr_t loader2(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  if (fd < 0) {
    Log("File not found: %s", filename);
    return 0;
  }
  assert(fd >= 2);

  // make sure the elf read is the fs_read
  Elf_Ehdr ehdr;

  assert(fs_read(fd, &ehdr, sizeof(ehdr)) == sizeof(ehdr));
  // replace the ramdisk_read with fs_read, it used to be zero while ramdisk_read.
  //ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  printf("[loader] file %d ehdr.e_ident: %x\n", fd, *(uint32_t *)ehdr.e_ident);

  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);

  //printf("ehdr.e_phnum: %d\n", ehdr.e_phnum);

  Elf_Phdr phdr[ehdr.e_phnum];

  assert(fs_lseek(fd, ehdr.e_ehsize, SEEK_SET) == ehdr.e_ehsize);
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

