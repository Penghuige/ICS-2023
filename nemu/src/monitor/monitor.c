/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

char *strtab = NULL; 
Elf32_Sym *symtab = NULL;
int num_symbols = 0;

static void welcome() {
#ifdef CONFIG_BATCHMODE
    return;
#endif
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
  // Log("Exercise: Please remove me in the source code and compile NEMU again.");
  // assert(0);// the problem code 
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static char *elf_file = NULL;
static int difftest_port = 1234;

static void read_section(int fd, off_t offset, size_t size, void *buf) {
    lseek(fd, offset, SEEK_SET);
    int a = read(fd, buf, size);
    assert(a != -1);
}

static void initial_table() {
    int fd = open(elf_file, O_RDONLY);
    if (fd < 0) {
        perror("open");
        assert(0);
    }

    // Read the ELF header
    unsigned char e_ident[EI_NIDENT];
    read_section(fd, 0, EI_NIDENT, e_ident);

    if (memcmp(e_ident, ELFMAG, SELFMAG) != 0) {
        fprintf(stderr, "Not an ELF file\n");
        close(fd);
        assert(0);
    }

    int is_64_bit = (e_ident[EI_CLASS] == ELFCLASS64);
    //int is_little_endian = (e_ident[EI_DATA] == ELFDATA2LSB);

    if (is_64_bit) {
        Elf64_Ehdr ehdr;
        read_section(fd, 0, sizeof(ehdr), &ehdr);

        Elf64_Shdr *shdrs = malloc(ehdr.e_shentsize * ehdr.e_shnum);
        read_section(fd, ehdr.e_shoff, ehdr.e_shentsize * ehdr.e_shnum, shdrs);

        char *shstrtab = malloc(shdrs[ehdr.e_shstrndx].sh_size);
        read_section(fd, shdrs[ehdr.e_shstrndx].sh_offset, shdrs[ehdr.e_shstrndx].sh_size, shstrtab);

        Elf64_Shdr *symtab_shdr = NULL;
        Elf64_Shdr *strtab_shdr = NULL;

        for (int i = 0; i < ehdr.e_shnum; i++) {
            const char *name = shstrtab + shdrs[i].sh_name;
            if (strcmp(name, ".symtab") == 0) {
                symtab_shdr = &shdrs[i];
            } else if (strcmp(name, ".strtab") == 0) {
                strtab_shdr = &shdrs[i];
            }
        }

        if (!symtab_shdr || !strtab_shdr) {
            fprintf(stderr, "No symbol table or string table found\n");
            free(shstrtab);
            free(shdrs);
            close(fd);
            assert(0);
        }

        symtab = malloc(symtab_shdr->sh_size);
        read_section(fd, symtab_shdr->sh_offset, symtab_shdr->sh_size, symtab);

        strtab = malloc(strtab_shdr->sh_size);
        read_section(fd, strtab_shdr->sh_offset, strtab_shdr->sh_size, strtab);

        num_symbols = symtab_shdr->sh_size / sizeof(Elf64_Sym);
        //for (int i = 0; i < num_symbols; i++) {
        //    Elf64_Sym *sym = &symtab[i];
        //    printf("Symbol: %s, Value: 0x%lx, Size: %lu\n",
        //           &strtab[sym->st_name], sym->st_value, sym->st_size);
        //}

        //free(strtab);
        //free(symtab);
        //free(shstrtab);
        //free(shdrs);
    } else {
        Elf32_Ehdr ehdr;
        read_section(fd, 0, sizeof(ehdr), &ehdr);

        Elf32_Shdr *shdrs = malloc(ehdr.e_shentsize * ehdr.e_shnum);
        read_section(fd, ehdr.e_shoff, ehdr.e_shentsize * ehdr.e_shnum, shdrs);

        char *shstrtab = malloc(shdrs[ehdr.e_shstrndx].sh_size);
        read_section(fd, shdrs[ehdr.e_shstrndx].sh_offset, shdrs[ehdr.e_shstrndx].sh_size, shstrtab);

        Elf32_Shdr *symtab_shdr = NULL;
        Elf32_Shdr *strtab_shdr = NULL;

        for (int i = 0; i < ehdr.e_shnum; i++) {
            const char *name = shstrtab + shdrs[i].sh_name;
            if (strcmp(name, ".symtab") == 0) {
                symtab_shdr = &shdrs[i];
            } else if (strcmp(name, ".strtab") == 0) {
                strtab_shdr = &shdrs[i];
            }
        }

        if (!symtab_shdr || !strtab_shdr) {
            fprintf(stderr, "No symbol table or string table found\n");
            free(shstrtab);
            free(shdrs);
            close(fd);
            assert(0);
        }

        symtab = malloc(symtab_shdr->sh_size);
        read_section(fd, symtab_shdr->sh_offset, symtab_shdr->sh_size, symtab);

        strtab = malloc(strtab_shdr->sh_size);
        read_section(fd, strtab_shdr->sh_offset, strtab_shdr->sh_size, strtab);

        num_symbols = symtab_shdr->sh_size / sizeof(Elf32_Sym);
        //for (int i = 0; i < num_symbols; i++) {
        //    Elf32_Sym *sym = &symtab[i];
        //    printf("Symbol: %s, Value: 0x%x, Size: %u, info: %d\n",
        //           &strtab[sym->st_name], sym->st_value, sym->st_size, sym->st_info);
        //}

        //free(strtab);
        //free(symtab);
        //free(shstrtab);
        //free(shdrs);
    }

    close(fd);
}

static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);

  Log("The image is %s, size = %d", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"ftrace"   , required_argument, NULL, 't'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:t:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 't': elf_file = optarg; initial_table(); break;
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\t-t,--ftrace=FILE        run ftrace\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

#ifndef CONFIG_ISA_loongarch32r
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv,
      MUXDEF(CONFIG_RV64,      "riscv64",
                               "riscv32"),
                               "bad"))) "-pc-linux-gnu"
  ));
#endif

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
