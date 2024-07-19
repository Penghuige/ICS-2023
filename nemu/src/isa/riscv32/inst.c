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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>
#include <elf.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

#define MCAUSE_ECALL_FROM_U 8
#define MCAUSE_ECALL_FROM_M 11
#define PRV_M 3

enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_J, TYPE_R, TYPE_B,
  TYPE_N, // none
};

static vaddr_t *csr_register(word_t imm)
{
  switch(imm)
  {
    case 0x300: return &(cpu.csr.mstatus);
    case 0x305: return &(cpu.csr.mtvec);
    case 0x341: return &(cpu.csr.mepc);
    case 0x342: return &(cpu.csr.mcause);
    default: panic("Bad csr!");
  }
}

#define src1R() do { *src1 = R(rs1); } while(0)
#define src2R() do { *src2 = R(rs2); } while(0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = SEXT((BITS(i, 31, 25) << 5) | BITS(i, 11, 7), 12); } while(0)
#define immB() do { *imm = SEXT((BITS(i, 31, 31) << 12) | (BITS(i, 7, 7) << 11) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1), 13); } while(0)
#define immJ() do { *imm = SEXT((BITS(i, 31, 31) << 20) | (BITS(i, 19, 12) << 12) | (BITS(i, 20, 20) << 11) | (BITS(i, 30, 21) << 1), 21); } while(0)
#define immR() do { /* No immediate value for R-type instructions */ } while(0)
#define ECALL(dnpc) do { dnpc = isa_raise_intr(cpu.csr.prv == PRV_M ? MCAUSE_ECALL_FROM_M:MCAUSE_ECALL_FROM_U, s->pc); } while(0)
#define CSR(i) *csr_register(i)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
	  case TYPE_J:                   immJ(); break;
	  case TYPE_R: src1R(); src2R(); immR(); break;
	  case TYPE_B: src1R(); src2R(); immB(); break;
  }
}
#ifdef CONFIG_STRACE
static void etrace_record(Decode *s)
{
  Log("exception occur at pc:%08x, exception NO:%d", s->pc, R(17));
}
static void etrace_return(Decode *s)
{
  Log("return from exception at pc:%08x, to %08x", s->pc, s->dnpc);
}
#else
static void etrace_record(Decode *s) {}
static void etrace_return(Decode *s) {}
#endif


#ifdef CONFIG_FTRACE
extern char* strtab;
extern Elf32_Sym* symtab;
extern int num_symbols;
int count = 0;

static void ftrace_record(Decode *s)
{
  // exec the dnpc
  // check which func is the dnpc going.
  for(int i = 0; i < num_symbols; i++)
  {
    Elf32_Sym *sym = &symtab[i];
    if(sym->st_info == 18 && \
        sym->st_value +sym->st_size > s->dnpc && s->dnpc >= sym->st_value)
    {
      // record the function
      Elf32_Sym * sym2 = NULL;
      for(int j = 0; j < num_symbols; j++)
      {
        sym2 = &symtab[j];
        if(sym2->st_info == 18 && \
             sym2->st_value + sym2->st_size > s->pc && s->pc >= sym2->st_value)
        {
          break;
        }
      }
      // not to save the loop
      if(sym == sym2) return;
      // assert(sym2 != NULL);
      if(sym2->st_info != 18)
      {
        // it is _start section
        for(int j = 0; j < num_symbols; j++)
        {
          sym2 = &symtab[j];
          if(sym2->st_info == 18 && sym2->st_size == 0) 
          {
            break;
          }
        }
      }
      if(s->dnpc == sym->st_value) count ++;
      else count--;
      printf("\033[0;31m[0x%8x]", s->pc);
      for(int j = 1; j < count; j++)
      {
        printf("  ");
      }
      printf(" From %s(0x%8x) to %s(0x%8x).\033[0m\n", \
          &strtab[sym2->st_name], s->pc, &strtab[sym->st_name], sym->st_value);
      break;
    }
  }
}
#else
static void ftrace_record(Decode *s)
{
  return;
}
#endif

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
} 

INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, if (src1 == src2) s->dnpc = imm + s->pc;ftrace_record(s));
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, if (src1 != src2) s->dnpc = imm + s->pc;ftrace_record(s));
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, if ((int)src1 <  (int)src2) s->dnpc = imm + s->pc;ftrace_record(s));
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, if ((int)src1 >= (int)src2) s->dnpc = imm + s->pc;ftrace_record(s));
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, if ((uint32_t)src1 <  (uint32_t)src2) s->dnpc = s->pc + imm;ftrace_record(s));
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, if ((uint32_t)src1 >= (uint32_t)src2) s->dnpc = s->pc + imm;ftrace_record(s));

  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(rd) = SEXT(Mr(src1 + imm, 1), 16));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(rd) = SEXT(Mr(src1 + imm, 2), 16));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1)); 
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(rd) = Mr(src1 + imm, 2));

  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0

  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm);
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(rd) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(rd) = src1 - src2);

  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(rd) = ((int)src1 < (int)src2) ? 1 : 0);
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(rd) = ((int)src1 < (int)imm) ? 1 : 0);
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(rd) = ((uint32_t)src1 < (uint32_t)src2) ? 1 : 0);

  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, R(rd) = ((int32_t)src1 >> src2));
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(rd) = ((uint32_t)src1 >> src2));
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(rd) = ((uint32_t)src1 << src2));

  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , I, R(rd) = ((uint32_t)src1 << imm));
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , I, R(rd) = ((uint32_t)src1 >> imm));
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   , I, R(rd) = ((int32_t)src1 >> imm));

  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(rd) = (src1 & imm));
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(rd) = (src1 | imm));
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(rd) = (src1 ^ imm));
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(rd) = src1 + imm);
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(rd) = ((uint32_t)src1 < (uint32_t)imm));

  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(rd) = (src1 ^ src2));
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(rd) = (src1 | src2));
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(rd) = (src1 & src2));

  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, R(rd) = s->pc + 4; s->dnpc = imm + s->pc; ftrace_record(s));
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, uint32_t t = s->pc + 4; s->dnpc = (src1 + imm) & ~1; R(rd) = t; ftrace_record(s));

  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(rd) = (src1 * src2));
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, int32_t a = src1; int32_t b = src2; int64_t tmp = (int64_t)a * (int64_t)b; R(rd) = BITS(tmp, 63, 32));
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu , R, uint64_t tmp = (int64_t)src1 * (uint64_t)src2; R(rd) = BITS(tmp, 63, 32));
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, uint64_t tmp = (uint64_t)src1 * (uint64_t)src2; R(rd) = BITS(tmp, 63, 32));
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(rd) = ((int32_t)src1 / (int32_t)src2));
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, R(rd) = ((uint32_t)src1 / (uint32_t)src2));
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, R(rd) = ((int32_t)src1 % (int32_t)src2));
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, R(rd) = ((uint32_t)src1 % (uint32_t)src2));

  INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs  , I, R(rd) = CSR(imm); CSR(imm) |= src1);
  INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw  , I, R(rd) = CSR(imm); CSR(imm) = src1);
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall  , I, etrace_record(s); ECALL(s->dnpc));
  // reset the previous prv, set the mpp to PRV_U(0), reset the previous mie to mpie, set mpie to 1
  INSTPAT("0011000 00010 00000 000 00000 11100 11", mret   , R, getchar(); s->dnpc = cpu.csr.mepc;cpu.csr.prv = cpu.csr.mpp, cpu.csr.mpp = 0, cpu.csr.mie = cpu.csr.mpie, cpu.csr.mpie = 1; etrace_return(s));

  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();
     
	//printf("\nsrc1:%08x imm:%d \n", src1, imm);
  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
