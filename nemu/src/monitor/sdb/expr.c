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
#include <memory/paddr.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
	TK_ADD,
	TK_SUB,
	TK_MUL,
	TK_DIV,
	TK_NUM,
	TK_BRAL,
	TK_BRAR,
  /* TODO: Add more token types */
	TK_NEQ,
	TK_AND,
	TK_HEX,
	TK_REG,
	TK_DER,
	TK_MOR,
	TK_LOW,
	TK_MOE,
	TK_LOE,
	TK_NEG,
	TK_POS,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" ", TK_NOTYPE},    	// spaces
  {"==", TK_EQ},        // equal
  {"\\+", TK_ADD},        // plus
	{"\\-", TK_SUB},				// subtract
	{"\\*", TK_MUL},				// multitute
	{"\\/", TK_DIV},				// divide
	{"\\b[0-9]+\\b", TK_NUM}, 	// number
	{"\\(", TK_BRAL},				// left quote
	{"\\)", TK_BRAR},				// right quote
	{"!=", TK_NEQ},
	{"&&", TK_AND},
	{"\\0x[0-9a-e]*", TK_HEX},
	{"\\$[a-zA-Z0-9]*", TK_REG},
	{">=", TK_MOE},
	{"<=", TK_LOE},
	{">", TK_MOR},
	{"<", TK_LOW},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

int pri[512] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

	// initial priority
	pri[TK_BRAR] = pri[TK_BRAL] = 1;
	pri[TK_NEG] = pri[TK_POS] = pri[TK_DER] = 2;
	pri[TK_MUL] = pri[TK_DIV] = 3;
	pri[TK_ADD] = pri[TK_SUB] = 4;
	pri[TK_MOR] = pri[TK_LOW] = pri[TK_MOE] = pri[TK_LOE] = 6;
	pri[TK_EQ] = pri[TK_NEQ] = 7;
	pri[TK_AND] = 11;
	pri[TK_NUM] = pri[TK_REG] = pri[TK_HEX] = 0;
	//pri[TK_]

	//printf("\nTK_MOR is %d , and TK_LOE is %d\n", TK_MOR, TK_LOE);

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

static Token tokens[65535] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

	// sign about the num is negative // it has been out
	//int sign = 0;
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
						if(nr_token == 0 || (tokens[nr_token-1].type != TK_NUM && tokens[nr_token-1].type != TK_BRAR) ) 
						{	
						  tokens[nr_token].type = TK_NEG;
						}
						else tokens[nr_token].type = TK_SUB;
						// if the sym is real sub
						strncpy(tokens[nr_token].str, substr_start, substr_len); 
						nr_token++;
						break;
					case TK_MUL:
						if(nr_token == 0 || (tokens[nr_token-1].type != TK_NUM && tokens[nr_token-1].type != TK_BRAR) ) 
						{						
						  tokens[nr_token].type = TK_DER;
							//printf("test1\n");
						}
						else tokens[nr_token].type = TK_MUL;
						// if the sym is real sub
						strncpy(tokens[nr_token].str, substr_start, substr_len); 
						nr_token++;
						break;
					case TK_ADD:
						if(tokens[nr_token-1].type != TK_NUM && tokens[nr_token-1].type != TK_BRAR ) 
						{						
						  tokens[nr_token].type = TK_POS;
							//printf("test1\n");
						}
						else tokens[nr_token].type = TK_ADD;
						// if the sym is real sub
						strncpy(tokens[nr_token].str, substr_start, substr_len); 
						nr_token++;
						break;
					case TK_REG:
						for(int i = 0; i <= substr_len; i++ )
						{
							if(*(substr_start+i) >= 'A' && *(substr_start+i) <= 'Z')
							{
								*(substr_start+i) -= 'A' + 'a';
							}
						}
					case TK_MOR:
					case TK_LOW:
					case TK_MOE:
					case TK_LOE:
					case TK_EQ:
					case TK_DIV:
          case TK_NUM:
					case TK_BRAL:
					case TK_BRAR:
					default:
						//if(sign == 1)
						//{
						//	// it is negative symbol
						//	tokens[nr_token].type = rules[i].token_type;
						//	sign = 0;
						//	if(rules[i].token_type == TK_NUM)
						//	{
						//		//num = -num;
						//		substr_start--;	
						//		substr_len++;
						//	}
						//}
						//if(sign == 2)
						//{
						//	sign = 0;	
						//	strncpy(tokens[nr_token].str, substr_start, substr_len); 
						//	bool sign_der = true;
						//	// printf("\n%s go to rederference.\n", tokens[nr_token].str);
						//	word_t ret = isa_reg_str2val(tokens[nr_token].str, &sign_der);
						//	//printf("%d", sign_der);
						//	if(sign_der == false)
						//	{
						//		printf("Can't find the register!\n");
						//		assert(0);
						//	}
						//	sprintf(tokens[nr_token].str, "%d", ret);
						//	nr_token++;
						//	break;
						//}
						tokens[nr_token].type = rules[i].token_type;
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
		if(tokens[i].type == TK_NUM) continue;
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
				printf("Bad expression 1!\n");
				assert(0);
			}
		}
	}
	if(to != 0)
	{
		printf("Bad expression 2!\n");
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
		if(tokens[p].type == TK_HEX)
		{
		 	// turn to hecimal

		 	uint32_t ret = strtol(tokens[p].str, NULL, 16);
			return ret;
		}
		else if(tokens[p].type == TK_REG)
		{
			bool sign = 1;
			uint32_t val = isa_reg_str2val(tokens[p].str, &sign);
			if(!sign)
			{
				printf("Can't find the register!\n");
				assert(0);
			}
			return val;
		}

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
		//int sign = 1;
	 	int ju = 0;
		int Max = 0;
		//printf("p:%d, q:%d \n", p, q);
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
			if(pri[tokens[i].type] >= Max)
			{
				//printf("%d::%d:%d:%s\n",pri[tokens[i].type], i, tokens[i].type, tokens[i].str);
				op = i;
				Max = pri[tokens[i].type];
			}
			//if(sign == 1 && (tokens[i].type == TK_MUL || tokens[i].type == TK_DIV))
			//{
			//	op = i;
			//}
		}
		//printf("op:%d\n", op);
		//for( i = p; i <= q; i++) {printf("%d:%s\n", i, tokens[i].str);}
		assert(op != -1);
		// to ensure what kind of the symbol is
    uint32_t val1 = 0;
    uint32_t val2 = 0; 
		if(tokens[op].type == TK_NEG || tokens[op].type == TK_POS || tokens[op].type == TK_DER)
		{
			val2 = eval(op + 1, q);
			// one parameter symbol
			//if(tokens[op].type == TK_DER)
			//{
			//	val2 = eval
			//}
			//printf("ok!\n");
		}
		else
		{
    	val1 = eval(p, op - 1);
    	val2 = eval(op + 1, q);
		}

    switch (tokens[op].type) {
      case TK_ADD: return val1 + val2;
      case TK_SUB: return val1 - val2;
      case TK_MUL: return val1 * val2;
      case TK_DIV: return val1 / val2;
			case TK_EQ: return val1 == val2; 
			case TK_NEQ: return val1 != val2;
			case TK_AND: return val1 && val2;
			case TK_NEG: return val1 - val2;
			case TK_POS: return val1 + val2;
			case TK_DER: return val1 + paddr_read(val2, 1);
			case TK_MOE: return val1 >= val2;
			case TK_LOE: return val1 <= val2;
			case TK_MOR: return val1 > val2;
			case TK_LOW: return val1 < val2;
      default: 
				//printf("tokens[op].str is %s\n", tokens[op].str);
				assert(0);

    }
  }
}	

void clear_exp()
{
	memset(tokens, 0, nr_token * sizeof(Token));
	nr_token = 0;
}
