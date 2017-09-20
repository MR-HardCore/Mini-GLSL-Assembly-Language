#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "common.h"
#include "parser.tab.h"
#include "ast.h"
#include "symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int semantic_check( node *ast);
void semantic_helper(node* cur);
void semantic_checker(node* cur);
int type_of_vector_element(int vec_type);


#endif