#include "ast.h"
#include "semantic.h"
#include "symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

int semantic_check(node *ast) {
    if (!ast) {
        /*report error here*/
        fprintf(errorFile, "Main scope not found.\n");
        return 0;
    } else {
        semantic_helper(ast);
        return 1;
    }
}

/* this helper function traverse the whole ast */
void semantic_helper(node* cur) {

    if (cur == NULL) {
        return;
    }
    //first, open scope 
    if (cur->kind == SCOPE_NODE) {
        enter_scope();
    }

    //second, go to recursion
    switch (cur->kind) {
        case SCOPE_NODE:
            semantic_helper(cur->scope.declarations);
            semantic_helper(cur->scope.statements);
            break;

        case DECLARATIONS_NODE:
            semantic_helper(cur->declarations.declarations);
            semantic_helper(cur->declarations.declaration);
            break;

        case DECLARATION_NODE:
            semantic_helper(cur->declaration.type_node);
            semantic_helper(cur->declaration.expr);
            break;

        case STATEMENTS_NODE:
            semantic_helper(cur->statements.statements);
            semantic_helper(cur->statements.statement);
            break;

        case UNARY_EXPRESION_NODE:
            semantic_helper(cur->unary_expr.right);
            break;

        case BINARY_EXPRESSION_NODE:
            semantic_helper(cur->binary_expr.left);
            semantic_helper(cur->binary_expr.right);
            break;

            //reached leaves
        case VAR_NODE:
            /* Do nothing */
            break;

        case INT_NODE:
            /* Do nothing */
            break;

        case FLOAT_NODE:
            /* Do nothing */
            break;

        case IDENT_NODE:
            /* Do nothing */
            break;

        case TYPE_NODE:
            /* Do nothing */
            break;

        case BOOL_NODE:
            /* Do nothing */
            break;

        case EXPR_VAR_NODE:
            semantic_helper(cur->expr_var_node.var_node);
            break;

        case STATEMENT_SCOPE_NODE:
            semantic_helper(cur->statement_scope_node.statement_scope_node);
            break;

        case EXPR_EXPR_NODE:
            semantic_helper(cur->expr_expr_node.expr);
            break;

        case IF_STATEMENT_NODE:
            semantic_helper(cur->if_statement.condition_expr);
            semantic_helper(cur->if_statement.if_block);
            semantic_helper(cur->if_statement.else_block);
            break;

        case ASSIGNMENT_NODE:
            semantic_helper(cur->assignment.variable);
            semantic_helper(cur->assignment.expr);
            break;

        case CONSTRUCTOR_NODE:
            semantic_helper(cur->constr.type_node);
            semantic_helper(cur->constr.args);
            break;

        case ARGUMENTS_NODE:
            semantic_helper(cur->args.args);
            semantic_helper(cur->args.expr);
            break;

        case FUNCTION_NODE:
            semantic_helper(cur->func.args);
            break;

        default:
            /* Do nothing */
            break;
    }

    //third, bottom up semantic chekck
    semantic_checker(cur);


};

void semantic_checker(node* cur) {

    node_kind kind = cur->kind;


    switch (kind) {

        case SCOPE_NODE:
            exit_scope();
            break;

        case DECLARATIONS_NODE:
            /* nothing to check here */
            break;

        case STATEMENTS_NODE:
            /* nothing to check here */
            break;

        case DECLARATION_NODE:
        {
            if (symbol_lookup_in_current_scope(cur->declaration.id)) {
                errorOccurred = 1;
                fprintf(errorFile, "Variable with ID %s is already declared in the scope.\n", cur->declaration.id);
                break;
            }

            /*compare the ID with predefined functions' names */
            if (
                    strcmp(cur->declaration.id, "gl_FragColor") == 0 ||
                    strcmp(cur->declaration.id, "gl_FragDepth") == 0 ||

                    strcmp(cur->declaration.id, "gl_FragCoord") == 0 ||
                    strcmp(cur->declaration.id, "gl_TexCoord") == 0 ||
                    strcmp(cur->declaration.id, "gl_Color") == 0 ||
                    strcmp(cur->declaration.id, "gl_Secondary") == 0 ||
                    strcmp(cur->declaration.id, "gl_FogFragCoord") == 0 ||

                    strcmp(cur->declaration.id, "gl_Light_Half") == 0 ||
                    strcmp(cur->declaration.id, "gl_Light_Ambient") == 0 ||
                    strcmp(cur->declaration.id, "gl_Material_Shininess") == 0 ||

                    strcmp(cur->declaration.id, "env1") == 0 ||
                    strcmp(cur->declaration.id, "env2") == 0 ||
                    strcmp(cur->declaration.id, "env3") == 0
                    ) {
                errorOccurred = 1;
                fprintf(errorFile, "Cannot redeclare a predefined variable %s\n", cur->declaration.id);
                break;
            }

            symbol_node new_node;

            /* When a declared variable is also assigned a value. */
            if (cur->declaration.expr) {
                //type mismatch
                if (cur->declaration.expr->type.type_code != -1 &&
                        !(cur->declaration.type_node->type.type_code == cur->declaration.expr->type.type_code &&
                        cur->declaration.type_node->type.vec == cur->declaration.expr->type.vec)) {
                    errorOccurred = 1;
                    fprintf(errorFile, "Declaration of %s, expecting type %s but got type %s\n",
                            cur->declaration.id,
                            get_type_str(&(cur->declaration.type_node->type)),
                            get_type_str(&(cur->declaration.expr->type)));

                }

                /*const var*/
                if (cur->declaration.is_const) {
                    if (!cur->declaration.expr->type.is_const) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Trying to initialize const variables without literals or uniform predefined variables.\n");
                    }
                }

                int num = cur->declaration.type_node->type.vec;

                while (num > 0) {

                    new_node.init_done[num - 1] = 1;
                    num--;

                }



            }/* Only declarations */
            else {
                new_node.init_done[0] = 0;
                new_node.init_done[1] = 0;
                new_node.init_done[2] = 0;
                new_node.init_done[3] = 0;
            }

            new_node.id = cur->declaration.id;
            new_node.is_const = cur->declaration.is_const;
            new_node.type_code = cur->declaration.type_node->type.type_code;
            new_node.vec = cur->declaration.type_node->type.vec;

            //fprintf(errorFile, "Saved %d\n", new_node.vec);
            symbol_insert(&new_node);

            //symbol_node *temp = symbol_lookup(cur->declaration.id);
            //fprintf(errorFile, "Fetched %d\n", temp->vec);
            break;
        }
        case ASSIGNMENT_NODE:
        {
            symbol_node *entry = symbol_lookup(cur->assignment.variable->var_node.id);
            /*Var Found*/
            if (entry) {

                /*array*/
                if (cur->assignment.variable->var_node.is_array) {

                    if (entry->is_const &&
                            (entry->init_done[cur->assignment.variable->var_node.index - 1])) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Attempting to change a const variable.\n");
                        break;
                    }
                }/*not an array*/
                else {
                    /*Check for changing a const already initialised*/
                    if (entry->is_const &&
                            ((entry->init_done[0]) || (entry->init_done[1]) ||
                            (entry->init_done[2]) || (entry->init_done[3]))) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Attempting to change a const variable.\n");
                        break;
                    }

                }

                /*Type check*/
                if (cur->assignment.expr->type.type_code != -1 && cur->assignment.variable->type.type_code != -1 &&
                        !(cur->assignment.variable->type.type_code == cur->assignment.expr->type.type_code &&
                        cur->assignment.variable->type.vec == cur->assignment.expr->type.vec)) {
                    errorOccurred = 1;
                    fprintf(errorFile, "Assignment of %s, expecting type: %s got type: %s\n",
                            cur->assignment.variable->var_node.id,
                            get_type_str(&(cur->assignment.variable->type)),
                            get_type_str(&(cur->assignment.expr->type)));
                    break;
                }




                /*Const assignment check, only literals and uniform value*/
                if (entry->is_const) {
                    if (!cur->assignment.expr->type.is_const) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Trying to initialize const variables without literals or uniform predefined variables.\n");
                        break;
                    }
                }


                /*Valid assignment*/

                if (cur->assignment.variable->var_node.is_array) {
                    //fprintf(errorFile, "ASI FETCHED AGAIN 1 %d \n", entry->vec);
                    //fprintf(errorFile, "index %d \n", cur->assignment.variable->var_node.index);
                    entry->init_done[cur->assignment.variable->var_node.index] = 1;
                    //fprintf(errorFile, "ASI FETCHED AGAIN 2 %d \n", entry->vec);
                } else {
                    entry->init_done[0] = 1;
                    entry->init_done[1] = 1;
                    entry->init_done[2] = 1;
                    entry->init_done[3] = 1;

                }

            }/*Not found in symbol table*/
            else {
                /*Predefined cases*/
                if (strcmp(cur->assignment.variable->var_node.id, "gl_FragColor") == 0 ||
                        strcmp(cur->assignment.variable->var_node.id, "gl_FragDepth") == 0
                        ) {


                    if (!is_main_scope()) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Result predefined variables can only be modified in the main scope\n");
                        break;
                    }


                    /*Type check*/
                    if (cur->assignment.expr->type.type_code != -1 && cur->assignment.variable->type.type_code != -1 &&
                            !(cur->assignment.variable->type.type_code == cur->assignment.expr->type.type_code &&
                            cur->assignment.variable->type.vec == cur->assignment.expr->type.vec)) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Assignment of %s, expecting type: %s got type: %s\n",
                                cur->assignment.variable->var_node.id,
                                get_type_str(&(cur->assignment.variable->type)),
                                get_type_str(&(cur->assignment.expr->type)));
                        break;
                    }

                    /*Valid assignment */

                } else {

                    if (strcmp(cur->assignment.variable->var_node.id, "gl_TexCoord") == 0 ||
                            strcmp(cur->assignment.variable->var_node.id, "gl_FragCoord") == 0 ||
                            strcmp(cur->assignment.variable->var_node.id, "gl_Color") == 0 ||
                            strcmp(cur->assignment.variable->var_node.id, "gl_Secondary") == 0 ||
                            strcmp(cur->assignment.variable->var_node.id, "gl_FogFragCoord") == 0 ||

                            strcmp(cur->assignment.variable->var_node.id, "gl_Light_Half") == 0 ||
                            strcmp(cur->assignment.variable->var_node.id, "gl_Light_Ambient") == 0 ||
                            strcmp(cur->assignment.variable->var_node.id, "gl_Material_Shininess") == 0 ||

                            strcmp(cur->assignment.variable->var_node.id, "env1") == 0 ||
                            strcmp(cur->assignment.variable->var_node.id, "env2") == 0 ||
                            strcmp(cur->assignment.variable->var_node.id, "env3") == 0) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Trying to modify a read-only predefined variable: %s.\n", cur->assignment.variable->var_node.id);
                        break;
                    }

                    errorOccurred = 1;
                    fprintf(errorFile, "Trying to assign an undeclared variable %s.\n", cur->assignment.variable->var_node.id);
                    break;
                }
            }


            break;
        }

        case IF_STATEMENT_NODE:
        {
            if (cur->if_statement.condition_expr->type.type_code != -1 &&
                    !(cur->if_statement.condition_expr->type.type_code == BOOL_T &&
                    cur->if_statement.condition_expr->type.vec == 1)) {
                errorOccurred = 1;
                fprintf(errorFile, "IF condition, expecting type: BOOL_T got type: %s\n",
                        get_type_str(&(cur->if_statement.condition_expr->type)));
                break;
            }

            break;
        }
        case STATEMENT_SCOPE_NODE:
            /* intermediate state */
            /* do nothing */
            break;


        case TYPE_NODE:
            /* leaf */
            /* do nothing */
            break;


        case CONSTRUCTOR_NODE:
        {


            /* Inherit type */
            cur->type.is_const = 1;
            cur->type.type_code = cur->constr.type_node->type.type_code;
            cur->type.vec = cur->constr.type_node->type.vec;

            int cur_vec = cur->type.vec;
            node *temp = cur->constr.args;


            if (temp == NULL) {
                errorOccurred = 1;
                fprintf(errorFile, "NULL input to constructor\n");
                break;
            }

            do {
                temp = temp->args.args;

                if (temp->type.is_const == 0) {
                    cur->type.is_const = 0;
                }

                if (temp->type.type_code != -1 &&
                        !(type_of_vector_element(cur->type.type_code) == temp->type.type_code &&
                        temp->type.vec == 1)) {

                    struct type_s temp_type;

                    temp_type.type_code = type_of_vector_element(cur->type.type_code);
                    temp_type.vec = 1;
                    errorOccurred = 1;
                    fprintf(errorFile, "Constructor cell at index %d: expecting type: %s got type: %s\n",
                            cur_vec - 1,
                            get_type_str(&(temp_type)),
                            get_type_str(&(temp->type)));
                }

                cur_vec--;

            } while (cur_vec > 0 && temp->args.args);

            /*Lack of arguments */
            if (cur_vec > 0) {
                errorOccurred = 1;
                fprintf(errorFile, "Constructor expecting %d arguments only got %d arguments.\n",
                        cur->type.vec, cur->type.vec - cur_vec);

            }

            /*Too many arguments */
            if (temp->args.args) {
                errorOccurred = 1;
                fprintf(errorFile, "Too arguments in constructor.\n");
            }

            /*Valid Inheritance*/

            break;
        }
        case FUNCTION_NODE:
        {

            cur->type.is_const = 0;

            int func = cur->func.name;

            if (func == 1) {
                cur->type.vec = 4;
            } else {
                cur->type.vec = 1;
            }

            if (cur->func.name == 0) {
                cur->type.type_code = -1;
            } else if (cur->func.name == 1) {
                cur->type.type_code = VEC_T;
            } else {
                cur->type.type_code = FLOAT_T;
            }


            node *temp = cur->func.args;

            /*NULL*/
            if (temp == NULL) {
                errorOccurred = 1;
                fprintf(errorFile, "Predefined functions expected a valid input\n");
                break;
            }

            /* Node with first valid expression */
            temp = temp->args.args;

            if (func == 0) {

                int type = temp->type.type_code;
                int vec = temp->type.vec;

                if (type != -1 &&
                        (!(type == VEC_T ||
                        type == IVEC_T) ||
                        !(vec == 3 ||
                        vec == 4))) {
                    errorOccurred = 1;
                    fprintf(errorFile, "Predefined function:dp3 argument count error\n");
                }

                temp = temp->args.args;

                if (type == VEC_T && (vec == 3 || vec == 4)) {
                    cur->type.type_code = FLOAT_T;

                    if (temp == NULL) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Predefined function:dp3 argument count error\n");
                        break;
                    }

                    /*Type matchup  check*/
                    if (temp->type.type_code != -1 &&
                            !(temp->type.type_code == type &&
                            temp->type.vec == vec)) {
                        struct type_s temp_type;
                        temp_type.type_code = type;
                        temp_type.vec = vec;
                        errorOccurred = 1;
                        fprintf(errorFile, "Function dp3 expecting 1st argument as type: %s, got type: %s\n",
                                get_type_str(&(temp_type)),
                                get_type_str(&(temp->type)));
                    }


                } else if (type == IVEC_T && (vec == 3 || vec == 4)) {
                    cur->type.type_code = INT_T;

                    if (temp == NULL) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Predefined function:dp3 argument count error\n");
                        break;
                    }

                    /*Type matchup  check*/
                    if (temp->type.type_code != -1 &&
                            !(temp->type.type_code == type &&
                            temp->type.vec == vec)) {
                        struct type_s temp_type;
                        temp_type.type_code = type;
                        temp_type.vec = vec;
                        errorOccurred = 1;
                        fprintf(errorFile, "Function dp3 expecting first argument as type: %s, getting type: %s\n",
                                get_type_str(&(temp_type)),
                                get_type_str(&(temp->type)));
                    }
                } else {

                    if (temp == NULL) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Predefined function:dp3 argument count error\n");
                        break;
                    }

                    if (temp->type.type_code != -1 &&
                            (!(temp->type.type_code == VEC_T ||
                            temp->type.type_code == IVEC_T) ||
                            !(temp->type.vec == 3 ||
                            temp->type.vec == 4))) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Predefined function:dp3 expecting first argument as type:IVEC_T or VEC_T with dimension of 3 or 4, getting type: %s\n",
                                get_type_str(&(temp->type)));
                    }

                }
                /* Too many arguments */
                if (temp->args.args) {
                    errorOccurred = 1;
                    fprintf(errorFile, "Predefined function:dp3 argument count error\n");
                }

            } else if (func == 1) {
                if (temp->type.type_code != -1 &&
                        !(temp->type.type_code == VEC_T &&
                        temp->type.vec == 4)) {
                    errorOccurred = 1;
                    fprintf(errorFile, "Predefined function:lit expecting argument as type:VEC_T with dimension of 4, getting type: %s\n",
                            get_type_str(&(temp->type)));
                }

                /* Too many arguments */
                if (temp->args.args) {
                    errorOccurred = 1;
                    fprintf(errorFile, "Predefined function:lit argument count error\n");
                }
            } else {
                int type = temp->type.type_code;
                int vec = temp->type.vec;

                if (type != -1 &&
                        (!(type == FLOAT_T || type == INT_T) ||
                        vec != 1)) {
                    errorOccurred = 1;
                    fprintf(errorFile, "Predefined function:rsq expecting it's argument as type:FLOAT_T or INT_T, getting type: %s\n",
                            get_type_str(&(temp->type)));
                }

                //temp = temp->args.args;


                /* Too many arguments */
                if (temp->args.args) {
                    errorOccurred = 1;
                    fprintf(errorFile, "Predefined function:lit argument count error\n");
                }
            }

            /*Valid*/
            break;


        }
        case UNARY_EXPRESION_NODE:
        {
            cur->type.is_const = 0;
            cur->type.vec = cur->unary_expr.right->type.vec;



            if (cur->unary_expr.op == '-') {/*'-'*/
                if (cur->unary_expr.right->type.type_code == BOOL_T || cur->unary_expr.right->type.type_code == BVEC_T) {
                    errorOccurred = 1;
                    fprintf(errorFile, "Unary operator -, expects an arithmetic type but got %s.\n",
                            get_type_str(&(cur->unary_expr.right->type)));
                    cur->type.type_code = -1;
                    break;

                }
                cur->type.type_code = cur->unary_expr.right->type.type_code;

            }/*'!'*/
            else {
                if (!(cur->unary_expr.right->type.type_code == BOOL_T || cur->unary_expr.right->type.type_code == BVEC_T) && cur->unary_expr.right->type.type_code != -1) {
                    errorOccurred = 1;
                    fprintf(errorFile, "Unary operator !, expects a logical type but got %s.\n",
                            get_type_str(&(cur->unary_expr.right->type)));
                    cur->type.type_code = -1;
                    break;

                }
                cur->type.type_code = cur->unary_expr.right->type.type_code;
            }

            break;
        }
        case BINARY_EXPRESSION_NODE:
        {
            cur->type.is_const = 0;

            int left_vec = cur->binary_expr.left->type.vec;
            int right_vec = cur->binary_expr.right->type.vec;

            cur->type.vec = (left_vec > right_vec) ?
                    left_vec : right_vec;

            int left_type = cur->binary_expr.left->type.type_code;
            int right_type = cur->binary_expr.right->type.type_code;
            int op = cur->binary_expr.op;

            /*Binary*/
            if (op == AND || op == OR) {
                /* if it is logical inputs */
                if ((left_type == -1 && type_of_vector_element(left_type) == BOOL_T) &&
                        (right_type == -1 && type_of_vector_element(right_type) == BOOL_T)) {

                    if (left_vec == right_vec) {

                        cur->type.type_code = (left_type == -1) ? right_type : left_type;

                    } else {
                        errorOccurred = 1;
                        fprintf(errorFile, "Binary operator %s, expects inputs of equal dimension but got %s and %s\n",
                                get_op_str(op),
                                get_type_str(&(cur->binary_expr.left->type)),
                                get_type_str(&(cur->binary_expr.right->type)));

                        cur->type.type_code = -1;
                    }

                } else {
                    errorOccurred = 1;
                    fprintf(errorFile, "Binary operator %s, expects logical type inputs but got %s and %s\n",
                            get_op_str(op),
                            get_type_str(&(cur->binary_expr.left->type)),
                            get_type_str(&(cur->binary_expr.right->type)));

                    cur->type.type_code = -1;
                }
            }/*Arithmetic*/
            else {
                if (type_of_vector_element(left_type) != BOOL_T && type_of_vector_element(right_type) != BOOL_T) {
                    if (op == '+' || op == '-') {
                        if (left_vec == right_vec && (left_type == -1 || right_type == -1 || left_type == right_type)) {
                            cur->type.type_code = left_type;

                        } else {
                            errorOccurred = 1;
                            fprintf(errorFile, "Binary operator %s, expects inputs of equal dimension and same type but got %s and %s\n",
                                    get_op_str(op),
                                    get_type_str(&(cur->binary_expr.left->type)),
                                    get_type_str(&(cur->binary_expr.right->type)));

                            cur->type.type_code = -1;
                        }
                    } else if (op == EQ || op == NEQ) {
                        if (left_vec == right_vec && left_type == -1 && right_type == -1 && left_type == right_type) {
                            cur->type.type_code = BOOL_T;
                        } else {
                            errorOccurred = 1;
                            fprintf(errorFile, "Binary operator %s, expects inputs of equal dimension and same type but got %s and %s\n",
                                    get_op_str(op),
                                    get_type_str(&(cur->binary_expr.left->type)),
                                    get_type_str(&(cur->binary_expr.right->type)));

                            cur->type.type_code = -1;
                        }

                    } else if (op == '*') {/*Operands Class SS VS SV VV : * */
                        if (left_type == -1 || right_type == -1 || type_of_vector_element(left_type) == type_of_vector_element(right_type)) {
                            cur->type.type_code = (left_vec > right_vec) ? left_type : right_type;
                        } else {
                            errorOccurred = 1;
                            fprintf(errorFile, "Binary operator %s, expects arguments of the same basic type but got %s and %s\n",
                                    get_op_str(op),
                                    get_type_str(&(cur->binary_expr.left->type)),
                                    get_type_str(&(cur->binary_expr.right->type)));
                            cur->type.type_code = -1;
                        }
                    } else {/*Operands Class SS : /, ^, <, <=, >, >= */
                        if (left_vec == 1 && right_vec == 1 &&
                                (left_type == -1 || right_type == -1 || left_type == right_type)) {

                            cur->type.type_code = (left_vec > right_vec) ? left_type : right_type;
                        } else {
                            errorOccurred = 1;
                            fprintf(errorFile, "Binary operator %s, expects arguments of the same type and dimension 1 but got %s and %s\n",
                                    get_op_str(op),
                                    get_type_str(&(cur->binary_expr.left->type)),
                                    get_type_str(&(cur->binary_expr.right->type)));
                            cur->type.type_code = -1;
                        }
                    }
                } else {/*Type does not match up*/
                    errorOccurred = 1;
                    fprintf(errorFile, "Binary operator %s, expects arithmetic type inputs but got %s and %s\n",
                            get_op_str(op),
                            get_type_str(&(cur->binary_expr.left->type)),
                            get_type_str(&(cur->binary_expr.right->type)));

                    cur->type.type_code = -1;
                }
            }
        }
            break;


        case BOOL_NODE:
        {
            /*leaf node*/
            cur->type.is_const = 1;
            cur->type.type_code = BOOL_T;
            cur->type.vec = 1;

            break;
        }
        case INT_NODE:
        {
            /*leaf node*/
            cur->type.is_const = 1;
            cur->type.type_code = INT_T;
            cur->type.vec = 1;

            break;
        }
        case FLOAT_NODE:
        {
            /*leaf node*/
            cur->type.is_const = 1;
            cur->type.type_code = FLOAT_T;
            cur->type.vec = 1;
            break;
        }
        case EXPR_EXPR_NODE:
        {
            /* Intermediate Node */
            cur->type.is_const = cur->expr_expr_node.expr->type.is_const;
            cur->type.type_code = cur->expr_expr_node.expr->type.type_code;
            cur->type.vec = cur->expr_expr_node.expr->type.vec;


            break;
        }
        case EXPR_VAR_NODE:
        {

            /* Intermediate Node */
            cur->type.is_const = cur->expr_var_node.var_node->type.is_const;
            cur->type.type_code = cur->expr_var_node.var_node->type.type_code;
            cur->type.vec = cur->expr_var_node.var_node->type.vec;


            break;

        }

        case VAR_NODE:
        {
            /*leaf node*/
            char *var_id = cur->var_node.id;

            /*Checking predefine variable */
            if (strcmp(var_id, "gl_FragColor") == 0 ||
                    strcmp(var_id, "gl_FragDepth") == 0) {
                //errorOccurred = 1;
                //fprintf(errorFile, "Trying to access a predefined write-only variable\n");
                cur->type.is_const = 0;
                cur->type.type_code = VEC_T;
                cur->type.vec = 4;
                if (cur->var_node.is_array) {
                    cur->type.type_code = FLOAT_T;
                    cur->type.vec = 1;
                }

                break;
            }

            if (strcmp(var_id, "gl_FragDepth") == 0) {

                cur->type.is_const = 0;
                cur->type.type_code = BOOL_T;
                cur->type.vec = 1;

                break;

            }

            /*Checking predefine variable */
            if (strcmp(var_id, "gl_FragCoord") == 0 ||
                    strcmp(var_id, "gl_TexCoord") == 0 ||
                    strcmp(var_id, "gl_Color") == 0 ||
                    strcmp(var_id, "gl_Secondary") == 0 ||
                    strcmp(var_id, "gl_FogFragCoord") == 0) {
                cur->type.is_const = 0;
                cur->type.type_code = VEC_T;
                cur->type.vec = 4;

                if (cur->var_node.is_array) {

                    cur->type.type_code = FLOAT_T;
                    cur->type.vec = 1;
                    if (cur->var_node.index < 0 || cur->var_node.index >= 4) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Array access  is out of bounds.\n");

                    }


                }

                break;
            }

            /*Checking predefine variable */
            if (strcmp(var_id, "gl_Light_Half") == 0 ||
                    strcmp(var_id, "gl_Light_Ambient") == 0 ||
                    strcmp(var_id, "gl_Material_Shininess") == 0 ||
                    strcmp(var_id, "env1") == 0 ||
                    strcmp(var_id, "env2") == 0 ||
                    strcmp(var_id, "env3") == 0) {
                cur->type.is_const = 1;
                cur->type.type_code = VEC_T;
                cur->type.vec = 4;
                if (cur->var_node.is_array) {
                    if (cur->var_node.index < 0 || cur->var_node.index >= 4) {
                        errorOccurred = 1;
                        fprintf(errorFile, "Array access is out of bounds.\n");

                    }
                    cur->type.type_code = FLOAT_T;
                    cur->type.vec = 1;
                }
                break;
            }




            /* Not a predefined variable */
            /*Check if symbol exists */
            symbol_node *entry = symbol_lookup(var_id);


            if (entry == NULL) {
                errorOccurred = 1;
                fprintf(errorFile, "Variable with id: %s has not been declared.\n",
                        var_id);

                cur->type.is_const = 0;
                cur->type.type_code = -1;
                cur->type.vec = 1;
                break;
            }

            //            if ((entry->init_done[0] == 0)&&(entry->init_done[1] == 0)&&
            //                    (entry->init_done[2] == 0)&&(entry->init_done[3] == 0)) {
            //                errorOccurred = 1;
            //                fprintf(errorFile, "Variable with id: %s has not been initialized.\n",
            //                        var_id);
            //            }

            if (cur->var_node.is_array) {
                if (cur->var_node.index < 0 || cur->var_node.index >= entry->vec) {
                    errorOccurred = 1;

                    if (cur->var_node.index) {
                        fprintf(errorFile, "%d\n", cur->var_node.index);
                    }
                    if (entry->vec) {
                        fprintf(errorFile, "%d\n", entry->vec);
                    }

                    fprintf(errorFile, "Array access is out of bounds.\n");

                }
                cur->type.vec = 1;
                cur->type.type_code = type_of_vector_element(entry->type_code);
            } else {
                cur->type.vec = entry->vec;
                cur->type.type_code = entry->type_code;
            }

            cur->type.is_const = entry->is_const;

            break;
        }

        case ARGUMENTS_NODE:
        {

            //char *var_id = cur->args.expr->var_node.id;
            //fprintf(errorFile, "%s\n", var_id);
            //symbol_node *tempo = symbol_lookup(var_id);

            //fprintf(errorFile, "ARGUS %d\n", tempo->vec);

            /* Intermediate Node */
            if (cur->args.expr) {
                cur->type.is_const = cur->args.expr->type.is_const;
                cur->type.type_code = cur->args.expr->type.type_code;
                cur->type.vec = cur->args.expr->type.vec;
            } else {
                cur->type.is_const = 0;
                cur->type.type_code = -1;
                cur->type.vec = 1;
            }

            break;
        }
        default: break;


    }
}

int type_of_vector_element(int vec_type) {
    if (vec_type == BOOL_T ||
            vec_type == FLOAT_T ||
            vec_type == INT_T)
        return vec_type;

    if (vec_type == BVEC_T)
        return BOOL_T;
    if (vec_type == VEC_T)
        return FLOAT_T;
    return INT_T;
}

