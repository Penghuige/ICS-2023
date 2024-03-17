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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
	TK_ADD = '+',
	TK_SUB = '-',
	TK_MUL = '*',
	TK_DIV = '/',
	TK_NUM,
	TK_BRAL = '(',
	TK_BRAR = ')',
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" ", TK_NOTYPE},    	// spaces
  {"\\b==\\b", TK_EQ},        // equal
  {"\\+", TK_ADD},        // plus
	{"\\-", TK_SUB},				// subtract
	{"\\*", TK_MUL},				// multitute
	{"\\/", TK_DIV},				// divide
	//{"0", TK_NUM}, 	// number
	{"\\b[0-9]+\\b", TK_NUM}, 	// number
	{"\\(", TK_BRAL},				// left quote
	{"\\)", TK_BRAR},				// right quote
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		printf("%d ", ret);
    if (ret != 0) {
			continue;
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

	// sign about the num is negative
	int sign = 0;
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
					case TK_NOTYPE:
						break;
					case TK_SUB:
						// consider the num may be a negative
						if(tokens[nr_token-1].type != TK_NUM) 
						{						
							sign ^= 1;
							break;
						}
						// if the sym is real sub
						tokens[nr_token].type = rules[i].token_type;
						strncpy(tokens[nr_token].str, substr_start, substr_len); 
						nr_token++;
						break;
					case TK_EQ:
					case TK_ADD:
					case TK_MUL:
					case TK_DIV:
          case TK_NUM:
					case TK_BRAL:
					case TK_BRAR:
					default:
						tokens[nr_token].type = rules[i].token_type;
						if(sign == 1)
						{
							// it is negative symbol
							sign = 0;
							if(rules[i].token_type == TK_NUM)
							{
								//num = -num;
								substr_start--;	
								substr_len++;
							}
						}
						strncpy(tokens[nr_token].str, substr_start, substr_len); 
						nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
	//for(int i = 0; i < nr_token; i++ ) 
		//printf("%d %s\n", i, tokens[i].str);

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
	else return nr_token;
}

bool check_parentheses(int p, int q)
{
	// set a stack to store the quote
	if(tokens[p].type != TK_BRAL || tokens[q].type != TK_BRAR)
		return false;
	int to = 0;
	
	int i;
	for(i = p; i <= q; i++)
	{
		if(tokens[i].type == TK_BRAL)
		{
			to++;
		}
		else if(tokens[i].type == TK_BRAR)
		{
			to--;
			if(i != q && to == 0) return false;
			if(to < 0) 
			{		
				printf("Bad expression!\n");
				assert(0);
			}
		}
	}
	if(to != 0)
	{
		printf("Bad expression!\n");
		assert(0);
	}
	else return true;
}
  /* TODO: Insert codes to evaluate the expression. */
uint32_t eval(uint32_t p, uint32_t q) {
  if (p > q) {
    /* Bad expression */
		printf("Bad expression!\n");
		assert(0);
  }
  else if (p == q) {
    /* Single token.
     */
		return atoi(tokens[p].str);
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    int op = -1; // = the position of 主运算符 in the token expression;
		int i;
		int sign = 1;
	 	int ju = 0;
		printf("p:%d, q:%d \n", p, q);
		for( i = p; i <= q; i++)
		{
			if(tokens[i].type == TK_BRAL)
			{
				ju = 1;
				continue;
			}
			else if (tokens[i].type == TK_BRAR)
			{
				ju = 0; 
				continue;
			}
			if(ju) continue;
			if(tokens[i].type == TK_ADD || tokens[i].type == TK_SUB)
			{
				op = i;
				sign = 0;
			}
			if(sign == 1 && (tokens[i].type == TK_MUL || tokens[i].type == TK_DIV))
			{
				op = i;
			}
		}
		assert(op != -1);
    uint32_t val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: assert(0);
    }
  }
}	

void clear_exp()
{
	memset(tokens, 0, nr_token * sizeof(Token));
	nr_token = 0;
}
