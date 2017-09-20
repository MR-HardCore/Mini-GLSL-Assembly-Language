#include "codegen.h"
#include "parser.tab.h"
int isleft=0;
char *leftname=NULL;

int inif=0;
int inelse=0;
int ifregid=-1;
int elseregid=-1;


void codegen(){
    printf("!!ARBfp1.0\n");
    reg_init();
    generate_helper(ast);
    printf("END\n");
}

int valid_ref(reg_reference ref){
    return (ref.reg_id!=-1)&&(ref.isvec>=0);
}

int is_predefined(char * name) {
    if (    (strcmp(name, "gl_FragColor") == 0) ||
            (strcmp(name, "gl_FragDepth") == 0) ||
            (strcmp(name, "gl_FragCoord") == 0) ||
            (strcmp(name, "gl_TexCoord") == 0) ||
            (strcmp(name, "gl_Color") == 0) ||
            (strcmp(name, "gl_Secondary") == 0) ||
            (strcmp(name, "gl_FogFragCoord") == 0) ||
            (strcmp(name, "gl_Light_Half") == 0) ||
            (strcmp(name, "gl_Light_Ambient") == 0) ||
            (strcmp(name, "gl_Material_Shininess") == 0) ||
            (strcmp(name, "env1") == 0) ||
            (strcmp(name, "env2") == 0) ||
            (strcmp(name, "env3") == 0)     ) {
        return 1;
    } else {
        return 0;
    }


}

char* translate_predefined(char * name) {
    if (strcmp(name, "gl_FragColor") == 0) {
        return "result.color";
    }
    else if (strcmp(name, "gl_FragDepth") == 0) {
        return "result.depth";
    }
    else if (strcmp(name, "gl_FragCoord") == 0) {
        return "fragment.position";
    }
    else if (strcmp(name, "gl_TexCoord") == 0) {
        return "fragment.texcoord";
    }
    else if (strcmp(name, "gl_Color") == 0) {
        return "fragment.color";
    }
    else if (strcmp(name, "gl_Secondary") == 0) {
        return "fragment.color.secondary";
    }
    else if (strcmp(name, "gl_FogFragCoord") == 0) {
        return "fragment.fogcoord";
    }
    else if (strcmp(name, "gl_Light_Half") == 0) {
        return "state.light[0].half";
    }
    else if (strcmp(name, "gl_Light_Ambient") == 0) {
        return "state.lightmodel.ambient";
    }
    else if (strcmp(name, "gl_Material_Shininess") == 0) {
        return "state.material.shininess";
    }
    else if (strcmp(name, "env1") == 0) {
        return "program.env[1]";
    }
    else if (strcmp(name, "env2") == 0) {
        return "program.env[2]";
    }
    else if (strcmp(name, "env3") == 0) {
        return "program.env[3]";
    }
    else{
        printf("Passing a non-predefined name!\n");
        return NULL;
    }
}

char index_to_char(int idx){
    switch(idx){
        case 0:
            return 'x';
            break;
        case 1:
            return 'y';
            break;
        case 2:
            return 'z';
            break;
        case 3:
            return 'w';
            break;
        default:
            printf("false char in index_to_char: %d\n",idx);
            assert(0);
            return 0;
    }
}





reg_reference generate_helper(node *cur){           //the actual generation routine. bottom up generation
    
    reg_reference right;
    right.reg_id=-1;
    right.isvec=-1;
    reg_reference left;
    left.reg_id=-1;
    left.reg_id=-1;
    
    if(cur==NULL){
        return right;
    }
    
    switch (cur->kind) {
        case SCOPE_NODE:                //done
            reg_enterscope();
            generate_helper(cur->scope.declarations);
            generate_helper(cur->scope.statements);
            reg_exitscope();
            return right;
            
            break;
            
        case UNARY_EXPRESION_NODE:{     //all reg_refs in this node must be temporary regs
            
            right=generate_helper(cur->unary_expr.right);
            if(!valid_ref(right)){printf("unary received a reg_id of -1!\n"); assert(0);}
            
            int op=cur->unary_expr.op;
            if(op=='-'){
                printf("MOV tempvar%d, -tempvar%d;\n",right.reg_id,right.reg_id);
            }
            else{       //'!'. we do -(k-1)
                printf("ADD tempvar%d, tempvar%d, {-1,-1,-1,-1};\n",right.reg_id,right.reg_id);
                printf("MOV tempvar%d, -tempvar%d;\n",right.reg_id,right.reg_id);
            }
            
            return right;
            break;
        }
        case BINARY_EXPRESSION_NODE:{       //all reg_refs in this node must be temporary regs
            
            left=generate_helper(cur->binary_expr.left);
            right=generate_helper(cur->binary_expr.right);
            
            if(!valid_ref(left)||!valid_ref(right)){printf("binary received a reg_id of -1!\n"); assert(0);}
            
            int op=cur->binary_expr.op;
            
            switch(op){
                case '+':{
                    printf("ADD tempvar%d, tempvar%d, tempvar%d;\n",left.reg_id,left.reg_id,right.reg_id);
                    break;
                }
                case '-':{
                    printf("ADD tempvar%d, tempvar%d, -tempvar%d;\n",left.reg_id,left.reg_id,right.reg_id);
                    break;
                }
                case '*':{
                    printf("MUL tempvar%d, tempvar%d, tempvar%d;\n",left.reg_id,left.reg_id,right.reg_id);
                    left.isvec=(left.isvec||right.isvec);
                    break;
                }
                case '/':{
                    if(right.isvec||left.isvec){printf("somehow we have a vec in divide operation. abort\n");assert(0);}
                    
                    printf("RCP tempvar%d, tempvar%d.x;\n",right.reg_id,right.reg_id);
                    printf("MUL tempvar%d, tempvar%d, tempvar%d;\n",left.reg_id,left.reg_id,right.reg_id);
                    break;
                }
                case '^':{
                    if(right.isvec||left.isvec){printf("somehow we have a vec in exp operation. abort\n");assert(0);}
                    
                    printf("POW tempvar%d,tempvar%d.x,tempvar%d.x;\n",left.reg_id,left.reg_id,right.reg_id);
                    break;
                }
                case AND:{      //perform l*r
                    printf("MUL tempvar%d, tempvar%d, tempvar%d;\n",left.reg_id,left.reg_id,right.reg_id);
                    break;
                }
                case OR:{       //perform -((l-1)*(r-1)-1)
                    printf("ADD tempvar%d,tempvar%d,-1;\n",left.reg_id,left.reg_id);
                    printf("ADD tempvar%d,tempvar%d,-1;\n",right.reg_id,right.reg_id);
                    printf("MUL tempvar%d, tempvar%d, tempvar%d;\n",left.reg_id,left.reg_id,right.reg_id);
                    printf("ADD tempvar%d,tempvar%d,-1;\n",left.reg_id,left.reg_id);
                    printf("MOV tempvar%d,-tempvar%d;\n",left.reg_id,left.reg_id);
                    break;
                }
                case EQ:{       //perform abs(abs(l-r)-1)
                    printf("MOV tempvar%d,-tempvar%d;\n",right.reg_id,right.reg_id);
                    printf("ADD tempvar%d,tempvar%d,tempvar%d;\n",left.reg_id,left.reg_id,right.reg_id);
                    printf("ABS tempvar%d,tempvar%d;\n",left.reg_id,left.reg_id);
                    printf("ADD tempvar%d,tempvar%d,-1;\n",left.reg_id,left.reg_id);
                    printf("ABS tempvar%d,tempvar%d;\n",left.reg_id,left.reg_id);
                    break;
                }
                case NEQ:{      //abs(l-r)
                    printf("MOV tempvar%d,-tempvar%d;\n",right.reg_id,right.reg_id);
                    printf("ADD tempvar%d,tempvar%d,tempvar%d;\n",left.reg_id,left.reg_id,right.reg_id);
                    printf("ABS tempvar%d,tempvar%d;\n",left.reg_id,left.reg_id);
                    break;
                }
                case '<':{      //l<r
                    printf("SLT tempvar%d,tempvar%d,tempvar%d;\n",left.reg_id,left.reg_id,right.reg_id);
                    break;
                }
                case LEQ:{      //l<=r == r>=l
                    printf("SGE tempvar%d,tempvar%d,tempvar%d;\n",left.reg_id,right.reg_id,left.reg_id);
                    break;
                }
                case '>':{      //l>r == r<l
                    printf("SLT tempvar%d,tempvar%d,tempvar%d;\n",left.reg_id,right.reg_id,left.reg_id);
                    break;
                }
                case GEQ:{      //l>=r
                    printf("SGE tempvar%d,tempvar%d,tempvar%d;\n",left.reg_id,left.reg_id,right.reg_id);
                    break;
                }
            }
            
            reg_free(right.reg_id);     //we always accumulate the result in left register.
            return left;
            
            break;
        }
        
        case BOOL_NODE:{
            int prev_alloc;
            left.reg_id=reg_allocate(&prev_alloc);
            left.isvec=0;
            if(!prev_alloc){
                printf("TEMP tempvar%d;\n",left.reg_id);
            }
            
            printf("MOV tempvar%d,{%d,%d,%d,%d};\n",left.reg_id,cur->bool_val,cur->bool_val,cur->bool_val,cur->bool_val);
            
            return left;
            break;    
        }
        case INT_NODE:{
            int prev_alloc;
            left.reg_id=reg_allocate(&prev_alloc);
            left.isvec=0;
            if(!prev_alloc){
                printf("TEMP tempvar%d;\n",left.reg_id);
            }
            
            printf("MOV tempvar%d,{%d,%d,%d,%d};\n",left.reg_id,cur->int_val,cur->int_val,cur->int_val,cur->int_val);
            
            return left;
            
            break;
        }
        
        case FLOAT_NODE:{
            int prev_alloc;
            left.reg_id=reg_allocate(&prev_alloc);
            left.isvec=0;
            if(!prev_alloc){
                printf("TEMP tempvar%d;\n",left.reg_id);
            }
            
            printf("MOV tempvar%d,{%f,%f,%f,%f};\n",left.reg_id,cur->float_val,cur->float_val,cur->float_val,cur->float_val);
            
            return left;
           
            break;
        }
        
        case VAR_NODE:{
            if(isleft){     //if it is on the left of an assignment, create a name for the target reference
                if(is_predefined(cur->var_node.id)){
                    char temp[200];
                    bzero(temp,200);
                    
                    if(cur->var_node.index==-1){
                        sprintf(temp,"%s",translate_predefined(cur->var_node.id));
                    }
                    else{
                        sprintf(temp,"%s.%c",translate_predefined(cur->var_node.id),index_to_char(cur->var_node.index));
                    }
                    
                    leftname=&temp[0];
                    
                }
                else{
                    int prev_id=reg_findsymbol(cur->var_node.id);
                    if(prev_id==-1){        //if there is no symbol thats impossible!
                        printf("var_node: there exist no var!\n");
                        assert(0);
                    }
                    
                    char temp[200];
                    bzero(temp,200);
                    
                    if(cur->var_node.index==-1){
                        sprintf(temp,"tempvar%d",prev_id);
                    }
                    else{
                        sprintf(temp,"tempvar%d.%c",prev_id,index_to_char(cur->var_node.index));
                    }
                    
                    leftname=&temp[0];
                    
                }
                isleft=0;               //there can be only one item on the left
            }
            else{           //else it is on the right side. we want its value
                if(is_predefined(cur->var_node.id)){
                    int prev_alloc;
                    left.reg_id=reg_allocate(&prev_alloc);
                    if(!prev_alloc){
                        printf("TEMP tempvar%d;\n",left.reg_id);
                    }
                    
                    
                    if(cur->var_node.index==-1){        //load the whole variable
                        printf("MOV tempvar%d,%s;\n",left.reg_id,translate_predefined(cur->var_node.id));
                        
                        if(strcmp("gl_FragDepth",cur->var_node.id)==0){     
                            left.isvec=0;
                        }
                        else{
                            left.isvec=1;
                        }
                        
                    }
                    else{                               //load the specific one
                        printf("MOV tempvar%d,%s.%c;\n",left.reg_id,translate_predefined(cur->var_node.id),index_to_char(cur->var_node.index));
                        left.isvec=0;
                    }
                }
                else{
                    int prev_id=reg_findsymbol(cur->var_node.id);
                    if(prev_id==-1){        //if there is no symbol thats impossible!
                        printf("var_node: there exist no var!\n");
                        assert(0);
                    }

                    if(cur->var_node.index==-1){        //referencing the whole variable
                        right.reg_id=prev_id;
                        right.isvec=cur->var_node.is_array;

                        int prev_alloc;
                        left.reg_id=reg_allocate(&prev_alloc);
                        left.isvec=right.isvec;
                        if(!prev_alloc){
                            printf("TEMP tempvar%d;\n",left.reg_id);
                        }

                        printf("MOV tempvar%d,tempvar%d;\n",left.reg_id,right.reg_id);
                        return left;
                    }
                    else{               //referencing a specific index
                        right.reg_id=prev_id;
                        right.isvec=0;

                        int prev_alloc;
                        left.reg_id=reg_allocate(&prev_alloc);
                        left.isvec=0;
                        if(!prev_alloc){
                            printf("TEMP tempvar%d;\n",left.reg_id);
                        }

                        printf("MOV tempvar%d,tempvar%d.%c;\n",left.reg_id,right.reg_id,index_to_char(cur->var_node.index));
                    }
                }  
            }
            
            return left;
            break;
        }
        
        case FUNCTION_NODE:{
            reg_reference arg1;
            reg_reference arg2;
            if(cur->func.name==0){      //dp3
                arg1=generate_helper(cur->func.args->args.args->args.expr);
                arg2=generate_helper(cur->func.args->args.args->args.args->args.expr);
                
                printf("DP3 tempvar%d, tempvar%d, tempvar%d;\n",arg1.reg_id,arg1.reg_id,arg2.reg_id);
                reg_free(arg2.reg_id);
            }
            else if(cur->func.name==1){ //lit 
                arg1=generate_helper(cur->func.args->args.args->args.expr);
                printf("LIT tempvar%d, tempvar%d;\n",arg1.reg_id,arg1.reg_id);
            }
            else{                       //rsq
                arg1=generate_helper(cur->func.args->args.args->args.expr);
                printf("RSQ tempvar%d, tempvar%d.x;\n",arg1.reg_id,arg1.reg_id);
            }
            
            left.reg_id=arg1.reg_id;
            left.isvec=arg1.isvec;
            return left;
            break;
        }
        
        case CONSTRUCTOR_NODE:{
            right=generate_helper(cur->constr.type_node);
            int i;
            
            int prev_alloc;
            left.reg_id=reg_allocate(&prev_alloc);
            left.isvec=1;       //only vectors are possible to be initialized
            if(!prev_alloc){
                printf("TEMP tempvar%d;\n",left.reg_id);
            }
            
            cur=cur->constr.args->args.args;
            for(i=0;i<right.reg_id;i++){
                reg_reference temp=generate_helper(cur->args.expr);
                cur=cur->args.args;
                printf("MOV tempvar%d.%c, tempvar%d;\n",left.reg_id,index_to_char(right.reg_id-1-i),temp.reg_id);
                reg_free(temp.reg_id);
            }
            
            return left;
            break;
        }
        case ARGUMENTS_NODE:
            //should never be here
            assert(0);
            return left;
            break;
        

        case TYPE_NODE:
            //!!!!important that the return value of type node is not a register, it is the dimension of the vector
            left.reg_id=cur->type.vec;
            return left;
            break;
        
        
        case EXPR_VAR_NODE:
            
            left=generate_helper(cur->expr_var_node.var_node);
            return left;
            break;

        case EXPR_EXPR_NODE:
            
            
            left=generate_helper(cur->expr_expr_node.expr);
            return left;     
            break;

        case STATEMENTS_NODE:
            
            generate_helper(cur->statements.statements);
            generate_helper(cur->statements.statement);
            
            return left;
            break;
            
        case IF_STATEMENT_NODE:{
            int prev_alloc;
            int ifcond=reg_allocate(&prev_alloc);
            if(!prev_alloc){
                printf("TEMP tempvar%d;\n",ifcond);
            }
            int elsecond=reg_allocate(&prev_alloc);
            if(!prev_alloc){
                printf("TEMP tempvar%d;\n",elsecond);
            }
            
            reg_reference cond=generate_helper(cur->if_statement.condition_expr);     //cond contains the value of 0 or 1, and must be a scalar
            
            if(cond.isvec){
                printf("in if_statement node condition is not a scalar\n");
                assert(0);
            }
            
            //set the two conditions 
            printf("ADD tempvar%d,tempvar%d,-1;\n",ifcond,cond.reg_id);     //cond=0,if=-1,cond=1,if=0
            printf("MOV tempvar%d, -tempvar%d;\n",elsecond,cond.reg_id);     //cond=0 else=0 cond=1 else=-1 
            reg_free(cond.reg_id);
            
            
            //now we have two condition temporaries avaliable,set the global variable
            ifregid=ifcond;
            elseregid=elsecond;
            
            inif=1;
            inelse=0;
            generate_helper(cur->if_statement.if_block);
            
            inif=0;
            inelse=1;
            generate_helper(cur->if_statement.else_block);
            
            inelse=0;
            
            return left;
            break;
        }
        case ASSIGNMENT_NODE:{
            
            isleft=1;
            generate_helper(cur->assignment.variable);
            right=generate_helper(cur->assignment.expr);
            
            if(leftname==NULL){
                printf("assignment node: name impossible to be NULL!\n");
                assert(0);
            }
            
            if(inif){
                printf("CMP tempvar%d,tempvar%d,%s,tempvar%d;\n",right.reg_id,ifregid,leftname,right.reg_id);
            }
            if(inelse){
                printf("CMP tempvar%d,tempvar%d,%s,tempvar%d;\n",right.reg_id,elseregid,leftname,right.reg_id);
            }
            
            if(right.isvec){
                printf("MOV %s,tempvar%d;\n",leftname,right.reg_id);
            }
            else{
                printf("MOV %s,tempvar%d;\n",leftname,right.reg_id);
            }
            
            reg_free(right.reg_id);
            
            return left;
            break;
        }
        
        case STATEMENT_SCOPE_NODE:
            
            generate_helper(cur->statement_scope_node.statement_scope_node);
            
            return left;
            break;
        
        case DECLARATIONS_NODE:
            
            generate_helper(cur->declarations.declarations);
            generate_helper(cur->declarations.declaration);
            
            return left;
            break;
            
        case DECLARATION_NODE:{
            
            left=generate_helper(cur->declaration.type_node);
            if(cur->declaration.expr!=NULL){
                right=generate_helper(cur->declaration.expr);
            }
            
            //we have a new symbol! make a register for it and add it to the reg symbol table 
            int prev_alloc;
            left.reg_id=reg_allocate(&prev_alloc);
            if(!prev_alloc){
                printf("TEMP tempvar%d;\n",left.reg_id);
            }
            
            reg_insertsymbol(cur->declaration.id,left.reg_id);
            
            if(cur->declaration.expr!=NULL){
                printf("MOV tempvar%d, tempvar%d;\n",left.reg_id,right.reg_id);
                reg_free(right.reg_id);
            }
            else{
                printf("MOV tempvar%d, 0;\n",left.reg_id);
            }
            
            return left;
            break;
        }
        default:
            printf("Reached default in codegen! This is bad!\n");
            break;
    }
    assert(0);
    return left;
}