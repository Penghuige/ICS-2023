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
#include <cpu/cpu.h>
#include <memory/paddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void wp_display();
void new_wp(char* exp);
void free_wp(int n);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args) ;

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_help(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Let the propram execute step N and then suspend execution", cmd_si },
  { "info", "print the state and info", cmd_info },
  { "x", "output the expression value" , cmd_x},
	{ "p", "eval the expression value", cmd_p},
	{ "w", "if the watching value changes, procedure will be halt", cmd_w},
	{ "d", "delete a watchpoint", cmd_d},

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
	char * arg = strtok(NULL, " ");
	int num;
	#ifdef CONFIG_WATCHPOINT
	//printf("hello?\n");
	#endif
	
	if (arg == NULL) {
	/* no argument given, it usaul execute one step*/
		cpu_exec(1);
	}
	else {
		num = atoi(arg);
		if (num == 0)
			printf("Parameter error!\n");
		else cpu_exec(num); 
	}

		return 0;
}
static int cmd_info(char * args) {
	char * arg = strtok(NULL, " ");
	if(arg == NULL) {
		printf("Need for argument!\n");
		return 0;
	}
//	printf("%s\n", arg);
//	if ( (int)(sizeof(arg)/arg[0]) != 2 )
//	{
//		printf("%d\n", (int)(sizeof(arg)/arg[0]));
//		printf("Parameter error!\n");
//		return 0;
//	}	
	if(strcmp(arg, "r") == 0) {
		isa_reg_display();
	}
	else if(strcmp(arg, "w") == 0) {
	 	wp_display();
	}
	return 0;
}

static int cmd_x(char *args)
{
	char* arg = strtok(NULL, " ");
	if(arg == NULL) 
	{
		printf("Need For Parameter!\n");
		return 0;
	}
	int n = atoi(arg);
	//printf("N is %d\n", n);
	if(n == 0) 
	{
		printf("Parameter Error!\n");
		return 0;
	}
	arg = strtok(NULL, " ");
	paddr_t addr;
	//if( strlen(arg) <= 2)
	if(arg[1] != 'x')
	{
		addr = (paddr_t)atoi(arg);
		//printf("Invalid parameter!\n");
		//return 0;
	}
	else
	{
  	addr = (paddr_t)strtol(arg+2, NULL, 16);
	}
	
	printf("%s\t\t%s\t\t\t\t\t%s\n", "addr","hex","dec");
	//printf("addr is %d\n", addr);	
	int i;
	for (i = 0; i < n; i++) {
		//printf("%d\n", isa_mmu_translate(addr + i, 4, 16));
		int j;
		printf("%x\t", addr+4*i);
		
		for(j = 0; j < 4; j++)
		{
			//output 4 steps, in order to output a 8 bytes address
			printf("%02x\t", paddr_read(addr+4*i + j, 1));  
		}
		printf("\t");
		for(j = 0; j < 4; j++)
		{	
			printf("%02d\t", paddr_read(addr+4*i  + j, 1));
		}
		printf("\n");
	}
	return 0;
}

static int cmd_p(char * args)
{
	// it already be a right string.
	// need to input with space !!!
	// to sure the input is right
	//printf("%s\n", args);
	if(args == NULL)
	{
		printf("Need for parameter!\n");
		return 0;
	}
	bool sign = true;
	word_t to = expr(args, &sign);
	if(sign == false)
	{
		printf("invalid parameter!\n");
		return 0;
	}
	
	uint32_t res = eval(0, to-1);
	clear_exp();
	printf("%s = %d\n", args, res);

	return 0;
}

static int cmd_w(char * args)
{
	new_wp(args);	
	return 0;
}
static int cmd_d(char * args)
{
	int a = atoi(args);
	free_wp(a);
	return 0;
}

// exec the batch mode
void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
#ifdef CONFIG_BATCHMODE
  sdb_set_batch_mode();
#endif
  if (!is_batch_mode) {
    cmd_c(NULL);
    return;
  }
/*

	FILE *fp = fopen("/home/penghui/ics2023/nemu/tools/gen-expr/build/input", "r");
	assert(fp != NULL);

	ssize_t nread;
	size_t len = 65536;
	char *test = (char*)malloc(len*sizeof(char));

	while((nread = getline(&test, &len, fp)) != -1)
	{
		// test is the test case
		char* res = strtok(test, " ");	
		char* exp = strtok(NULL, "\n");	
		//char* exp = strtok(NULL, "\n");	
		uint32_t resNum = atoi(res);
		printf("%s\n", exp);
		cmd_p(exp);
		if(resNum == -123) return;
	}
	fclose(fp);
	if(1) return;
*/

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
