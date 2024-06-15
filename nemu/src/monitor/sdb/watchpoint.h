#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

void init_wp_pool();

void new_wp(char* exp);

void free_wp(int n);

void wp_display();

int check_wp();

#endif
