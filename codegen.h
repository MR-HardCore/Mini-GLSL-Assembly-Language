#ifndef CODEGEN_H__
#define CODEGEN_H__

#include "ast.h"
#include "reg_table.h"
#include "common.h"
#include <assert.h>

extern node *ast;

typedef struct reg_ref__{
    int reg_id;
    int isvec;      
}reg_reference;

void codegen();     //the generator
reg_reference generate_helper(node *n);  //the actual recursive generation routine

#endif