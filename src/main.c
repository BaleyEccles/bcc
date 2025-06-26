#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "AST.h"
#include "graph.h"

void generate_default_types(dynamic_array* ts)
{
    // TODO more types
    type* type_void = malloc(sizeof(type));
    type_void->string = "void";
    da_append(ts, type_void, type*);
    type* type_int = malloc(sizeof(type));
    type_int->string = "int";
    da_append(ts, type_int, type*);
    type* type_bool = malloc(sizeof(type));
    type_bool->string = "bool";
    da_append(ts, type_bool, type*);
    type* type_float = malloc(sizeof(type));
    type_float->string = "float";
    da_append(ts, type_float, type*);
}



void generate_template_asm(FILE* file, AST_node* root)
{
    fprintf(file, ".section .text\n");
    fprintf(file, ".globl _start\n");
    fprintf(file, "\n");
    fprintf(file, "_start:\n");
    fprintf(file, "    pushq %%rbp\n");
    fprintf(file, "    movq %%rsp, %%rbp\n");
    fprintf(file, "\n");
}

void generate_rvalue_asm(FILE* file, AST_node* scope, AST_node* node)
{
    if (node->node_type == OPERATOR) {
        AST_node* child1 = ((AST_node**)node->children->data)[0];
        AST_node* child2 = ((AST_node**)node->children->data)[1];
        
        //generate_rvalue_asm(file, scope, child2);
        //fprintf(file, "    addl $%s, %%eax\n", ((constant*)child1->data)->value);
        
        TOKEN_TYPE t = ((operator*)node->data)->type;
        switch (t) {
        case PLUS: {
            generate_rvalue_asm(file, scope, child2);
            if (child1->node_type == CONSTANT) {
                fprintf(file, "    addl $%s, %%eax\n", ((constant*)child1->data)->value);
            } else {
                fprintf(file, "    addl -%i(%%rbp), %%eax\n", ((varible*)child1->data)->stack_pos);
            }
            break;
        }
        case GREATER_THAN: {
            generate_rvalue_asm(file, scope, child1);
            fprintf(file, "    movl %%eax, %%esi\n");
            generate_rvalue_asm(file, scope, child2);
            fprintf(file, "    movl %%eax, %%edi\n");
            fprintf(file, "    cmpl %%edi, %%esi\n");
            fprintf(file, "    jg ");
            break;
        }
        case GREATER_THAN_OR_EQUALS: {
            generate_rvalue_asm(file, scope, child1);
            fprintf(file, "    movl %%eax, %%esi\n");
            generate_rvalue_asm(file, scope, child2);
            fprintf(file, "    movl %%eax, %%edi\n");
            fprintf(file, "    cmpl %%edi, %%esi\n");
            fprintf(file, "    jge ");
            break;
        }
        case LESS_THAN: {
            generate_rvalue_asm(file, scope, child1);
            fprintf(file, "    movl %%eax, %%esi\n");
            generate_rvalue_asm(file, scope, child2);
            fprintf(file, "    movl %%eax, %%edi\n");
            fprintf(file, "    cmpl %%edi, %%esi\n");
            fprintf(file, "    jl ");
            break;
        }
        case LESS_THAN_OR_EQUALS: {
            generate_rvalue_asm(file, scope, child1);
            fprintf(file, "    movl %%eax, %%esi\n");
            generate_rvalue_asm(file, scope, child2);
            fprintf(file, "    movl %%eax, %%edi\n");
            fprintf(file, "    cmpl %%edi, %%esi\n");
            fprintf(file, "    jge ");
            break;
        }
        case LOGICAL_AND: {
            generate_rvalue_asm(file, scope, child1);
            fprintf(file, "AND_%i\n", node->token->pos_in_file);
            fprintf(file, "    jmp AND_false_%i\n", node->token->pos_in_file);
            
            fprintf(file, "AND_%i:\n", node->token->pos_in_file);
            generate_rvalue_asm(file, scope, child2);
            fprintf(file, "AND_true_%i\n", node->token->pos_in_file);
            fprintf(file, "    jmp AND_false_%i\n", node->token->pos_in_file);
            
            fprintf(file, "AND_false_%i:\n", node->token->pos_in_file);
            fprintf(file, "    movl $0, %%eax\n");
            fprintf(file, "    jmp AND_end_%i\n", node->token->pos_in_file);
            fprintf(file, "AND_true_%i:\n", node->token->pos_in_file);
            fprintf(file, "    movl $1, %%eax\n");
            fprintf(file, "    jmp AND_end_%i\n", node->token->pos_in_file);
            
            fprintf(file, "AND_end_%i:\n", node->token->pos_in_file);
            fprintf(file, "    cmpl $1, %%eax\n");
            fprintf(file, "    je ");

            break;
        }
        default: {
        }
            fprintf(stderr, "%s:%d: TODO: Operator %s %i was not handled\n", __FILE__, __LINE__, node->token->data, node->token->pos_in_file);
            fprintf(file  , "%s:%d: TODO: Operator %s %i was not handled\n", __FILE__, __LINE__, node->token->data, node->token->pos_in_file);
        }
        
        
    }
    if (node->node_type == VARIBLE) {
        fprintf(file, "    movl -%i(%%rbp), %%eax\n", ((varible*)node->data)->stack_pos);
    }
    if (node->node_type == CONSTANT) {
        fprintf(file, "    movl $%s, %%eax\n", ((constant*)node->data)->value);
    }
}



void generate_asm_from_node(FILE* file, AST_node* scope, AST_node* node);
void create_asm_file(FILE* file, AST_node* scope, AST_node* root);

    
void generate_if_asm(FILE* file, AST_node* scope, AST_node* node)
{
    if (node->children->count == 3) {
        AST_node* else_node = ((AST_node**)node->children->data)[2];
        
        generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
        fprintf(file, "%s_%i\n", node->token->data, node->token->pos_in_file);
        fprintf(file, "    jmp %s_%i\n", else_node->token->data, else_node->token->pos_in_file);
 
        fprintf(file, "%s_%i:\n", node->token->data, node->token->pos_in_file);
        generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
        fprintf(file, "    jmp end_%s_%i\n", node->token->data, node->token->pos_in_file);
        
        fprintf(file, "%s_%i:\n", else_node->token->data, else_node->token->pos_in_file);
        generate_asm_from_node(file, scope, else_node);
    } else if (node->children->count == 2) {

        generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
        fprintf(file, "%s_%i\n", node->token->data, node->token->pos_in_file);
        
        fprintf(file, "%s_%i:\n", node->token->data, node->token->pos_in_file);
        generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
        fprintf(file, "    jmp end_%s_%i\n", node->token->data, node->token->pos_in_file);
    }
    fprintf(file, "end_%s_%i:\n", node->token->data, node->token->pos_in_file);
    
}

void generate_asm_from_node(FILE* file, AST_node* scope, AST_node* node)
{
    if (node->node_type == OPERATOR) {
        generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[1]);
        fprintf(file, "    movl %%eax, -%i(%%rbp)\n", ((varible*)((AST_node**)node->children->data)[0]->data)->stack_pos);
        fprintf(file, "\n");
    } else if (node->node_type == KEY_WORD) {
        key_word* kw = (key_word*)node->data;
        switch (kw->key_word_type) {
        case RETURN: {
            // We need to do different things if we are returning from main or a user function
            if (scope->node_type == FUNCTION) {
                if (strcmp(((function*)scope->data)->name, "main") == 0) {
                    // Do syscall return
                    generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
                    fprintf(file, "    movl %%eax, %%edi\n");
                    fprintf(file, "    movl $60, %%eax\n");
                    fprintf(file, "    syscall\n");
                }
            }
            else {
                fprintf(stderr, "%s:%d: TODO: Function return is not handled\n", __FILE__, __LINE__);
            }
            break;
        }
        case IF: {
            generate_if_asm(file, scope, node);
            break;
        }
        case ELSE: {
            generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[0]);
            break;
        }
        default: {
            fprintf(stderr, "%s:%d: TODO: Key word '%s' was not handled\n", __FILE__, __LINE__, kw->name);
            break;
        }
        }
    }

}


void create_asm_file(FILE* file, AST_node* scope, AST_node* root)
{
    if (root->node_type == FUNCTION) {
        if (strcmp(((function*)root->data)->name, "main") != 0) {
            fprintf(stderr, "%s:%d: error: root node is not the main function\n", __FILE__, __LINE__);
        }
        else if (strcmp(((function*)root->data)->name, "main") == 0) {
            generate_template_asm(file, root);
        }
    }
    
    for (int i = 0; i < root->children->count; i++) {
        AST_node* node = ((AST_node**)root->children->data)[i];
        generate_asm_from_node(file, scope, node);
    }
}

int main()
{
    char input_name[] = "./tests/if_statement/if_statement.c";
    //char name[] = "./tests/test1/test1.c";
    FILE* input_file = fopen(input_name, "r");
    
    fseek(input_file, 0L, SEEK_END);
    int input_file_size = ftell(input_file);
    rewind(input_file);
    
    char c;
    while((c = getc(input_file)) != EOF) {
        printf("%c", c);
    }

    int pos = 0;
    dynamic_array tokens;
    da_init(&tokens, token*);
        
    while (pos < input_file_size) {
        token* t = get_next_token(input_file, &pos);
        da_append(&tokens, t, token*);
    }

    // Generate default types
    dynamic_array ts;
    da_init(&ts, type);
    generate_default_types(&ts);
    
    clean_tokens(&tokens);
    for (int i = 0; i < tokens.count; i++) {
        ((token**)tokens.data)[i]->type = get_token_type(((token**)tokens.data)[i], &ts);
        printf("token with index %i at %i: %s and type %i\n", i,  ((token**)tokens.data)[i]->pos_in_file, ((token**)tokens.data)[i]->data, ((token**)tokens.data)[i]->type);
    }

    
    // Generate AST

    AST_node* main_node = create_main_function_node(&tokens);
        
    int main_token_location = get_token_location(&tokens, main_node->token);
    int main_inputs_token_end = get_closing_paren_location(&tokens, main_token_location + 1);
    int start = main_inputs_token_end + 2;
    int end = get_closing_paren_location(&tokens, start - 1) - 1;
    
    main_node = create_body_AST_node(main_node, main_node, &tokens, start, end);
    generate_stack_posistions(main_node, 0);
    

    generate_graphviz_from_AST_node(main_node, "graph.gv");
    
    FILE* asm_file = fopen("output.asm", "w");
    create_asm_file(asm_file, main_node, main_node);
    fclose(asm_file);
    
    asm_file = fopen("output.asm", "r");
    printf("\nAssembly file:\n\n");
    while ((c = getc(asm_file)) != EOF) {
        printf("%c", c);
    }

    return 0;
}
