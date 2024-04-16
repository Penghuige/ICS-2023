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

#include "sdb.h"
#include <memory/paddr.h>

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
	word_t val;
	paddr_t addr;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

// int count_wp = 0;
void new_wp(paddr_t addr)
{
	if(free_ == NULL)
	{
		printf("The wp pool is full!\n");
		assert(0);
	}
	free_->addr = addr;
	free_->val = paddr_read(addr, 1);
	if(head == NULL) head = free_;
	else 
	{
		free_->next = head->next;
		head->next = free_;
	}
	free_  = free_->next;
}

void free_wp(int n)
{
	WP* temp = head;
	if(temp->NO == n)
	{
		free_->next = temp;
		head = NULL;
		return;
	}
	WP* bef = head;
	while(temp != NULL && temp->NO != n)
	{
		bef = temp;
		temp = temp->next;
	}
	if(temp == NULL)
	{
		printf("Can't find the watch point!\n");
		assert(0);
	}
	else
	{
		bef->next = temp->next;
		temp->next = free_->next;
		if(free_ != NULL) free_->next = temp;
		else free_ = temp;
	}
}
