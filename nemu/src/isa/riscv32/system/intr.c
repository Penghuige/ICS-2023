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

// here include isa.h, that can use the cpu
#include <isa.h>

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  cpu.csr.mcause = NO;
  // false instruction
  cpu.csr.mepc = epc;

  // mstatus
  cpu.csr.mpp = cpu.csr.prv;
  cpu.csr.prv = 3;

  cpu.csr.mpie = cpu.csr.mie;
  cpu.csr.mie = 0;

  return cpu.csr.mtvec;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
