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

	uint32_t val;
	char exp[100];
	word_t to;
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
void new_wp(char* exp)
{
	if(free_ == NULL)
	{
		printf("The wp pool is full!\n");
		assert(0);
	}
	WP* des = free_->next;
	strcpy(free_->exp, exp);
	bool sign = true;

	word_t to = expr(exp, &sign);
	if(sign == false)
	{
		printf("invalid parameter!\n");
		return;
	}
	free_->to = to;
	free_->val = eval(0, to-1); 
	clear_exp();
	if(head == NULL) 
	{
		head = free_;
		head->next = NULL;
	}
	else 
	{
		// head insert
		free_->next = head->next;
		head->next = free_;
	}
	free_ = des;
}

void free_wp(int n)
{
	WP* temp = head;
	if(temp->NO == n)
	{
		free_->next = temp;
		head = head->next;
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
		//printf("bef is %d, temp is %d\n", bef->NO, temp->NO);
		bef->next = temp->next;
		temp->next = free_->next;
		if(free_ != NULL) free_->next = temp;
		else free_ = temp;
	}
}

void wp_display()
{
	WP* temp = head;
	printf("NUM\t\tVAL\t\t\tEXP\t\t\n");
	while(temp != NULL)
	{
		printf("%d\t\t%08x\t\t\t%s\t\t\n", temp->NO, temp->val, temp->exp);
		temp = temp->next;
	}
	//printf("I am ok!\n");
}

int check_wp()
{
	WP* temp = head;
	while(temp != NULL)
	{
		bool sign = true;
		word_t to = expr(temp->exp, &sign);
		if(sign == false)
		{
			printf("Bad in check_wp.\n");
			assert(0);
		}
		uint32_t res = eval(0, to-1);
		if(res != temp->val)
		{
			printf("Previous:%x New:%x\n", temp->val, res);
			temp->val = res;
			return temp->NO;
		}
		temp = temp->next;
	}
	return -1;
}
