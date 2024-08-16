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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  bool sign = true;
  if(cpu.csr.mepc != ref_r->csr.mepc){
    printf("\e[1;31mthe different register is mepc, dut is %08x, ref is %08x\n", cpu.csr.mepc, ref_r->csr.mepc);
    sign = false;
  }
  if(cpu.csr.mtvec != ref_r->csr.mtvec){
    printf("\e[1;31mthe different register is mtvec, dut is %08x, ref is %08x\n", cpu.csr.mtvec, ref_r->csr.mtvec);
    sign = false;
  }
  if(cpu.csr.mstatus != ref_r->csr.mstatus){
    printf("\e[1;31mthe different register is mstatus, dut is %08x, ref is %08x\n", cpu.csr.mstatus, ref_r->csr.mstatus);
    sign = false;
  }
  if(cpu.csr.mcause != ref_r->csr.mcause){
    printf("\e[1;31mthe different register is mcause, dut is %08x, ref is %08x\n", cpu.csr.mcause, ref_r->csr.mcause);
    sign = false;
  }
  for(int i = 0; i < ARRLEN(ref_r->gpr); i++)
  {
    if(cpu.gpr[i] != ref_r->gpr[i]){
      extern const char *regs[];
      printf("\e[1;31mthe different register is regs %s, dut is %08x, ref is %08x\n\e[0m", regs[i], cpu.gpr[i], ref_r->gpr[i]);
      sign = false;
      break;
    }
  }
  if(!sign)
  {
     // disaplay all the right register
     printf("\e[1;31m$pc\t0x%x\n\e[0m", ref_r->pc);
     extern const char *regs[];
     for(int j = 0; j < ARRLEN(ref_r->gpr)/4; j++)
     {
       for(int k = 0; k < 4; k++)
       {
         printf("\e[1;31m$%s\t0x%08x\t\e[0m", regs[4*j+k], ref_r->gpr[4*j+k]);
       }
       printf("\n");
     }
     printf("\e[1;31mmtvec\t0x%08x\tmepc\t0x%08x\tmcause\t0x%08x\tmstatus\t0x%08x\n\e[0m", ref_r->csr.mtvec, ref_r->csr.mepc, ref_r->csr.mcause, ref_r->csr.mstatus);
  }
  // all new
  return sign;
}

void isa_difftest_attach() {
}
