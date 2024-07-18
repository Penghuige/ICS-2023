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
  if(NO == -1) 
  {
    // yield value is -1, but event_yield is 1
    //NO = 1; // event_yield
    //epc += 4;
  }
  cpu.csr.mcause = NO;
  // false instruction
  cpu.csr.mepc = epc;

  cpu.csr.mstatus |= ((1 << 12) + (1 << 11));

  return cpu.csr.mtvec;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
