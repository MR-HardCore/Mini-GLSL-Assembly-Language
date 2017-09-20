#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"

symbol_node *table=NULL;

void enter_scope(){
    symbol_node *temp=(symbol_node *)malloc(sizeof(symbol_node));        //this will be a head node rather than an actual symbol
    temp->id=NULL;
    temp->parent=table;
    temp->content=NULL;
    table=temp;
}
void exit_scope(){                          //destroy the current scope and return to the last scope
    symbol_node *current=table->content;
    while(current){
        symbol_node *temp=current;
        current=current->content;
        free(temp);
    }
    
    current=table;
    table=table->parent;
    free(current);
}
void symbol_insert(symbol_node *entry){
    symbol_node *temp=(symbol_node *)malloc(sizeof(symbol_node));
    temp->id=entry->id;
    int i;
    for(i=0;i<4;i++){
        temp->init_done[i]=entry->init_done[i];
    }
    temp->is_const=entry->is_const;
    temp->type_code=entry->type_code;
    temp->vec=entry->vec;
    
    temp->content=table->content;
    table->content=temp;
    temp->parent=NULL;         //symbol nodes should not have parent. only the parent node has.
}
symbol_node *symbol_lookup(char *symbol){       //continuously looking for the symbol until it is found or we reached the end of scope.
    symbol_node *current_scope=table;
    while(current_scope){
        symbol_node *current_symbol=current_scope->content;
        while(current_symbol){
            if(strcmp(current_symbol->id,symbol)==0){
                return current_symbol;
            }
            current_symbol=current_symbol->content;
        }
        current_scope=current_scope->parent;
    }
    
    return NULL;
}

symbol_node *symbol_lookup_in_current_scope(char *symbol){      //used by declaration node
    symbol_node *current_scope=table;
    symbol_node *current_symbol=current_scope->content;
    while(current_symbol){
        if(strcmp(current_symbol->id,symbol)==0){
            return current_symbol;
        }
        current_symbol=current_symbol->content;
    }
 
    return NULL;
}

int is_main_scope(){
    return table->parent==NULL;
}

void print_node(symbol_node *n){                            //only for debugging purpose
    if(!n){
        printf("node does not exist\n");
        return;
    }
    printf("id: %s,typecode: %d\n",n->id,n->type_code);
}