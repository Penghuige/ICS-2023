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
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

#define REG_NUM ARRLEN(regs)

// gpr is the register of NEMU, it located /nemu/src/isa/riscv32/include/isa-def.h
void isa_reg_display() {
	int i;	
	printf("$pc\t0x%x\n", cpu.pc);
	for (i = 0; i < REG_NUM/4; i++)
	{
		int j = 0;
		for(j = 0; j < 4; j++)
		{
			printf("$%s\t0x%08x\t", regs[4*i+j], cpu.gpr[4*i+j]);
		}
		printf("\n");
	}
  printf("mtvec\t0x%08x\tmepc\t0x%08x\tmcause\t0x%08x\tmstatus\t0x%08x\n", cpu.csr.mtvec, cpu.csr.mepc, cpu.csr.mcause, cpu.csr.mstatus);
}


word_t isa_reg_str2val(const char *s, bool *success) {
	int i = 0;
	if(strcmp(s, "pc") == 0 || strcmp(s, "$pc") == 0)
	{
		return cpu.pc;
	}
	for(i = 0; i < REG_NUM; i++)
	{
		if(strcmp(s+1, regs[i]) == 0) break;
	}
	if(i == REG_NUM)
	{
		*success = false;
		return -1;
	}
  return cpu.gpr[i];
}
