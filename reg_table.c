#include "reg_table.h"

reg_table rt;

void reg_init(){
    bzero(rt.alloc_list,REG_TABLE_SIZE*sizeof(int));
    bzero(rt.prev_alloced,REG_TABLE_SIZE*sizeof(int));
    rt.table=NULL;
}

int reg_allocate(int *prev_allocated){
    int i=0;
    for(i=0;i<REG_TABLE_SIZE;i++){
        if(rt.alloc_list[i]==0){
            *prev_allocated=rt.prev_alloced[i];
            rt.alloc_list[i]=1;
            rt.prev_alloced[i]=1;
            return i;
        }
    }
    printf("reg_allocate failed!\n");
    return -1;
}


void reg_free(int id){
    if(id<0 || id>=REG_TABLE_SIZE){
        printf("reg_free tried to free out of bound registers!\n");
    }
    if(rt.alloc_list[id]==0){
        printf("reg_free tried to free an unallocated reg!\n");
    }
    rt.alloc_list[id]=0;
}

int reg_findsymbol(char *sym){
    reg_symbol_node *current_scope=rt.table;
    while(current_scope){
        reg_symbol_node *current_symbol=current_scope->content;
        while(current_symbol){
            if(strcmp(current_symbol->symbol,sym)==0){
                return current_symbol->reg_id;
            }
            current_symbol=current_symbol->content;
        }
        current_scope=current_scope->parent;
    }
    
    return -1;
}

void reg_insertsymbol(char *sym, int id){
    reg_symbol_node *temp=(reg_symbol_node *)malloc(sizeof(reg_symbol_node));
    temp->reg_id=id;
    temp->symbol=sym;
    
    temp->content=rt.table->content;
    rt.table->content=temp;
    temp->parent=NULL;
}

void reg_enterscope(){
    reg_symbol_node *temp=(reg_symbol_node *)malloc(sizeof(reg_symbol_node));        //this will be a head node rather than an actual symbol
    temp->symbol=NULL;
    temp->reg_id=-1;
    temp->parent=rt.table;
    temp->content=NULL;
    rt.table=temp;
}

void reg_exitscope(){
    reg_symbol_node *current=rt.table->content;
    while(current){
        reg_symbol_node *temp=current;
        current=current->content;
        reg_free(temp->reg_id);
        free(temp);
    }
    
    current=rt.table;
    rt.table=rt.table->parent;
    free(current);
}

void reg_print(){
    printf("allocation list: {");
    int i=0;
    for(i=0;i<REG_TABLE_SIZE;i++){
        if(rt.alloc_list[i]){
            printf("%d,",i);
        }
    }
    printf("}\n");
}