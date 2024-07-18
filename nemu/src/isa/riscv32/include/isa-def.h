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

#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include <common.h>

typedef struct {
  word_t mtvec; // 它保存发生异常时处理器需要跳转到的地址。
  vaddr_t mepc; // 它指向发生异常的指令。
  word_t mcause; // （Machine Exception Cause）它指示发生异常的种类。
  union
  {
    // mstatus 需要更改很多状态才能迎合difftest的实现
    word_t mstatus; //（Machine Status）它保存全局中断使能，以及许多其他的状态，如
    // occupy 13 bytes, only the behind 13 bytes be used
    struct
    {
      word_t : 3;
      word_t mie : 1;
      word_t : 3;
      word_t mpie : 1;
      word_t : 3;
      word_t mpp : 2;
    };
  };
  int prv;
//} MUXDEF(CONFIG_RV64, riscv64_CSRs, riscv32_CSRs);
} riscv32_CSRs;

// register value located here
typedef struct {
  word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
  vaddr_t pc;
  riscv32_CSRs csr;
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);

// decode
typedef struct {
  union {
    uint32_t val;
  } inst;
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);

#define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)

#endif
