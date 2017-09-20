#ifndef _SYMBOL_H
#define _SYMBOL_H

typedef struct sn_{
    char *id;
    int is_const;
    int type_code;
    int vec;
    int init_done[4];       //we need this for checking initialization of vector entries
    struct sn_ *parent;         //pointer to the last scope. NULL when it is the main scope
    struct sn_ *content;        //pointer to other entries in the scope
}symbol_node;

void enter_scope();
void exit_scope();
void symbol_insert(symbol_node *entry);
symbol_node *symbol_lookup(char *symbol);
symbol_node *symbol_lookup_in_current_scope(char *symbol);
int is_main_scope();


extern symbol_node *table;

#endif

