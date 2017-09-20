
#ifndef REG_TABLE_H__

#define REG_TABLE_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define REG_TABLE_SIZE 1000

typedef struct reg_sn_{
    char *symbol;
    int reg_id;
    struct reg_sn_ *parent;         //pointer to the last scope. NULL when it is the main scope
    struct reg_sn_ *content;        //pointer to other entries in the scope
}reg_symbol_node;

typedef struct reg_t_{          //reg table consists of reg allocation list and a special symbol table
    int alloc_list[REG_TABLE_SIZE];
    int prev_alloced[REG_TABLE_SIZE];
    reg_symbol_node *table;
}reg_table;

extern reg_table rt;

void reg_init();
int reg_allocate(int *prev_allocated);     //return an unallocated temp reg id. if previously allocated then prev_alloc=1
void reg_free(int id);        //free an allocated temp reg id
int reg_findsymbol(char *s);   //find if there exist a symbol. if yes, return its reg id, else return -1
void reg_insertsymbol(char *s, int reg_id);    //insert an symbol to symbol table with its reg
void reg_enterscope();
void reg_exitscope();

void reg_print();       //for debugging purpose

#endif