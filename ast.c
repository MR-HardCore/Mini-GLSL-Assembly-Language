#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0
#define INDENT_PER_LEVEL 4

node *ast = NULL;

const char *get_type_str(struct type_s *type) {
  switch(type->type_code) {
    case FLOAT_T:
      return "FLOAT";
    case INT_T:
      return "INT";
    case BOOL_T:
      return "BOOL";
    case BVEC_T:
      switch(type->vec){
        case 2:
          return "BVEC2";
        case 3:
          return "BVEC3";
        case 4:
          return "BVEC4";
      }
    case IVEC_T:
      switch(type->vec){
        case 2:
          return "IVEC2";
        case 3:
          return "IVEC3";
        case 4:
          return "IVEC4";
      }
    case VEC_T:
      switch(type->vec){
        case 2:
          return "VEC2";
        case 3:
          return "VEC3";
        case 4:
          return "VEC4";
      }
    default:
      return "ANY";
  }
}

const char *get_op_str(int op) {
  switch(op) {
    case '-':
      return "-";
    case '!':
      return "!";
    case AND:
      return "&&";
    case OR:
      return "||";
    case EQ:
      return "==";
    case NEQ:
      return "!=";
    case '<':
      return "<";
    case LEQ:
      return "<=";
    case '>':
      return ">";
    case GEQ:
      return ">=";
    case '+':
      return "+";
    case '*':
      return "*";
    case '/':
      return "/";
    case '^':
      return "^";
    default:
      return "";
  }
}

const char *get_func_str(int name) {
  switch(name) {
    case 0:
      return "DP3";
    case 1:
      return "RSQ";
    case 2:
      return "LIT";
    default:
      return "";
  }
}



node *ast_allocate(node_kind kind, ...) {
    va_list args;

    // make the node
    node *ast = (node *) malloc(sizeof (node));
    memset(ast, 0, sizeof *ast); //mmk
    ast->kind = kind;

    va_start(args, kind);

    switch (kind) {

        case TYPE_NODE:
            ast->type.type_code = va_arg(args, int);
            ast->type.vec = va_arg(args, int)+1;
            break;

        case SCOPE_NODE:
            ast->scope.declarations = va_arg(args, node *); //Could be NULL.
            ast->scope.statements = va_arg(args, node *); //Could be NULL.
            break;

        case DECLARATIONS_NODE:
            ast->declarations.declarations = va_arg(args, node *); //Could be NULL.
            ast->declarations.declaration = va_arg(args, node *);
            break;

        case DECLARATION_NODE: //Note, create symbol table will be done after whole tree is initialised
            ast->declaration.is_const = va_arg(args, int);
            ast->declaration.id = va_arg(args, char *);
            ast->declaration.type_node = va_arg(args, node *);
            ast->declaration.expr = va_arg(args, node *); //Could be NULL.
            break;

        case STATEMENTS_NODE:
            ast->statements.statements = va_arg(args, node *); //Could be NULL.
            ast->statements.statement = va_arg(args, node *); //Could be NULL.
            break;

        case UNARY_EXPRESION_NODE:
            ast->unary_expr.op = va_arg(args, int);
            ast->unary_expr.right = va_arg(args, node *);
            break;

        case BINARY_EXPRESSION_NODE:
            ast->binary_expr.op = va_arg(args, int);
            ast->binary_expr.left = va_arg(args, node *);
            ast->binary_expr.right = va_arg(args, node *);
            break;

        case VAR_NODE:
            ast->var_node.id = va_arg(args, char *);
            ast->var_node.is_array = va_arg(args, int);
            ast->var_node.index = va_arg(args, int);
            break;

        case EXPR_VAR_NODE:
            ast->expr_var_node.var_node = va_arg(args, node *);
            break;

        case STATEMENT_SCOPE_NODE:
            ast->statement_scope_node.statement_scope_node = va_arg(args, node *);
            break;

        case EXPR_EXPR_NODE:
            ast->expr_expr_node.expr = va_arg(args, node *);
            break;

        case INT_NODE:
            ast->type.is_const = 1;
            ast->type.type_code = INT_T;
            ast->type.vec = 1;
            ast->int_val = va_arg(args, int);
            break;

        case FLOAT_NODE:
            ast->type.is_const = 1;
            ast->type.type_code = FLOAT_T;
            ast->type.vec = 1;
            ast->float_val = va_arg(args, double);
            break;

        case BOOL_NODE:
            ast->type.is_const = 1;
            ast->type.type_code = BOOL_T;
            ast->type.vec = 1;
            ast->bool_val = va_arg(args, int);
            break;

        case IF_STATEMENT_NODE:
            ast->if_statement.condition_expr = va_arg(args, node *);
            ast->if_statement.if_block = va_arg(args, node *); //Could be NULL.
            ast->if_statement.else_block = va_arg(args, node *); //Could be NULL.
            break;

        case ASSIGNMENT_NODE:
            ast->assignment.variable = va_arg(args, node *);
            ast->assignment.expr = va_arg(args, node *);
            break;

        case CONSTRUCTOR_NODE:
            ast->constr.type_node = va_arg(args, node *);
            ast->constr.args = va_arg(args, node *); //Could be NULL.
            break;

        case ARGUMENTS_NODE:
            ast->args.args = va_arg(args, node *); //Could be NULL
            ast->args.expr = va_arg(args, node *); //Could be NULL
            break;



        case FUNCTION_NODE:
            ast->func.name = va_arg(args, int);
            ast->func.args = va_arg(args, node *); //Could be NULL.
            break;


        




        default: 
            printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            break; //Error?
    }

    va_end(args);

    return ast;
}

void ast_print_helper(node *cur, int depth) {
    if(cur==NULL){
        return;
    }
    char *indentation=(char *)malloc(depth*INDENT_PER_LEVEL+1);
    int i;
    for(i=0;i<depth*INDENT_PER_LEVEL;i++){
        indentation[i]=' ';
    }
    indentation[i]=0;
    
    

    fprintf(dumpFile, indentation);
    
    fprintf(dumpFile, "(");
    
    switch (cur->kind) {
        case SCOPE_NODE:
            fprintf(dumpFile, "SCOPE\n");
            ast_print_helper(cur->scope.declarations,depth+1);
            ast_print_helper(cur->scope.statements,depth+1);
            break;
            
        case UNARY_EXPRESION_NODE:
            fprintf(dumpFile, "UNARY %s %s\n", get_type_str(&cur->type), get_op_str(cur->unary_expr.op));
            ast_print_helper(cur->unary_expr.right,depth+1);

            break;
        
        case BINARY_EXPRESSION_NODE:
            fprintf(dumpFile, "BINARY %s %s\n", get_type_str(&cur->type), get_op_str(cur->binary_expr.op));
            ast_print_helper(cur->binary_expr.left,depth+1);
            ast_print_helper(cur->binary_expr.right,depth+1);
            
            break;
        
        case INT_NODE:
            fprintf(dumpFile, "%d\n", cur->int_val);

            break;
         
        case FLOAT_NODE:
            fprintf(dumpFile, "%f\n", cur->float_val);

            break;

        case VAR_NODE:
            if (cur->var_node.is_array) {
                fprintf(dumpFile, "INDEX %s %s %d\n", get_type_str(&cur->type), cur->var_node.id,
                        cur->var_node.index);
            } else {
                fprintf(dumpFile, "%s\n", cur->var_node.id);
            }

            break;

        case FUNCTION_NODE:
            fprintf(dumpFile, "CALL %s\n", get_func_str(cur->func.name));
            ast_print_helper(cur->func.args,depth+1);
            break;
        
        case CONSTRUCTOR_NODE:
            fprintf(dumpFile, "CALL\n");
            ast_print_helper(cur->constr.type_node, depth+1);
            ast_print_helper(cur->constr.args, depth+1);
            break;
        
        case ARGUMENTS_NODE:
            //fprintf(dumpFile, "ARGUMENTS\n");
            fprintf(dumpFile, "\n");
            ast_print_helper(cur->args.args,depth+1);
            ast_print_helper(cur->args.expr,depth+1);
            
            break;
        

        case TYPE_NODE:
            fprintf(dumpFile, "%s\n", get_type_str(&cur->type));

            break;
        
        case BOOL_NODE:
            if (cur->bool_val) {
                fprintf(dumpFile, "true\n");
            } else {
                fprintf(dumpFile, "false\n");
            }

            break;

        case EXPR_VAR_NODE:
            //fprintf(dumpFile, "EXPR_VAR_NODE\n");
            fprintf(dumpFile, "\n");
            ast_print_helper(cur->expr_var_node.var_node,depth+1);

            break;

        case EXPR_EXPR_NODE:
            //fprintf(dumpFile, "EXPR_EXPR_NODE\n");
            fprintf(dumpFile, "\n");
            ast_print_helper(cur->expr_expr_node.expr,depth+1);

            break;

        case STATEMENTS_NODE:
            fprintf(dumpFile, "STATEMENTS\n");
            ast_print_helper(cur->statements.statements,depth+1);
            ast_print_helper(cur->statements.statement,depth+1);
            break;
            
        case IF_STATEMENT_NODE:
            fprintf(dumpFile, "IF\n");
            ast_print_helper(cur->if_statement.condition_expr,depth+1);
            ast_print_helper(cur->if_statement.if_block,depth+1);
            ast_print_helper(cur->if_statement.else_block,depth+1);
            break;
            
        case ASSIGNMENT_NODE:
            fprintf(dumpFile, "ASSIGNMENT %s\n", get_type_str(&cur->assignment.type));
            ast_print_helper(cur->assignment.variable,depth+1);
            ast_print_helper(cur->assignment.expr,depth+1);
            
            break;
            
        case STATEMENT_SCOPE_NODE:
            //fprintf(dumpFile, "STATEMENT_SCOPE_NODE\n");
            fprintf(dumpFile, "\n");
            ast_print_helper(cur->statement_scope_node.statement_scope_node,depth+1);
            break;
        
        case DECLARATIONS_NODE:
            fprintf(dumpFile, "DECLARATIONS\n");
            ast_print_helper(cur->declarations.declarations,depth+1);
            ast_print_helper(cur->declarations.declaration,depth+1);
            break;
            
        case DECLARATION_NODE:
            fprintf(dumpFile, "DECLARATION %s\n", cur->declaration.id); 
            ast_print_helper(cur->declaration.type_node,depth+1);
            ast_print_helper(cur->declaration.expr,depth+1);
            
            break;

        default:
            fprintf(dumpFile, "error: unknown node reached!\n");
            break;
    }

    fprintf(dumpFile, indentation);
    fprintf(dumpFile, ")");
    fprintf(dumpFile, "\n");
    
    free(indentation);
}


void ast_free_helper(node *cur) {
    if(cur==NULL){
        return;
    }
    switch (cur->kind) {
        case SCOPE_NODE:

            ast_free_helper(cur->scope.declarations);
            ast_free_helper(cur->scope.statements);
            break;
            
        case UNARY_EXPRESION_NODE:
            ast_free_helper(cur->unary_expr.right);

            break;
        
        case BINARY_EXPRESSION_NODE:
            ast_free_helper(cur->binary_expr.left);
            ast_free_helper(cur->binary_expr.right);
            
            break;
        
        case INT_NODE:
            

            break;
         
        case FLOAT_NODE:
            

            break;

        case VAR_NODE:
            free(cur->var_node.id);

            break;

        case FUNCTION_NODE:
            
            ast_free_helper(cur->func.args);
            break;
        
        case CONSTRUCTOR_NODE:
            
            ast_free_helper(cur->constr.type_node);
            ast_free_helper(cur->constr.args);
            break;
        
        case ARGUMENTS_NODE:
            
            
            ast_free_helper(cur->args.args);
            ast_free_helper(cur->args.expr);
            
            break;
        

        case TYPE_NODE:
            

            break;
        
        case BOOL_NODE:
            

            break;

        case EXPR_VAR_NODE:
            
            
            ast_free_helper(cur->expr_var_node.var_node);

            break;

        case EXPR_EXPR_NODE:
            
            
            ast_free_helper(cur->expr_expr_node.expr);

            break;

        case STATEMENTS_NODE:
            
            ast_free_helper(cur->statements.statements);
            ast_free_helper(cur->statements.statement);
            break;
            
        case IF_STATEMENT_NODE:
            
            ast_free_helper(cur->if_statement.condition_expr);
            ast_free_helper(cur->if_statement.if_block);
            ast_free_helper(cur->if_statement.else_block);
            break;
            
        case ASSIGNMENT_NODE:
            
            ast_free_helper(cur->assignment.variable);
            ast_free_helper(cur->assignment.expr);
            
            break;
            
        case STATEMENT_SCOPE_NODE:
            
            ast_free_helper(cur->statement_scope_node.statement_scope_node);
            break;
        
        case DECLARATIONS_NODE:
            
            ast_free_helper(cur->declarations.declarations);
            ast_free_helper(cur->declarations.declaration);
            break;
            
        case DECLARATION_NODE:
            free(cur->declaration.id);
            ast_free_helper(cur->declaration.type_node);
            ast_free_helper(cur->declaration.expr);
            
            break;

        default:
      
            break;
    }

    free(cur);
}

void ast_print(node * ast) {
    ast_print_helper(ast,0);
}


void ast_free(node *ast) {
    ast_free_helper(ast);
}
