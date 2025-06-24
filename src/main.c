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

void compute_rvalue(FILE* file, AST_node* node)
{
    if (node->node_type == OPERATOR) {

        AST_node* child1 = ((AST_node**)node->children->data)[0];
        AST_node* child2 = ((AST_node**)node->children->data)[1];
        compute_rvalue(file, child2);
        fprintf(file, "    addl $%s, %%eax\n", ((constant*)child1->data)->value);
        
    }
    if (node->node_type == CONSTANT) {
        fprintf(file, "    movl $%s, %%eax\n", ((constant*)node->data)->value);
    }
}


void create_asm_file(FILE* file, AST_node* root)
{
    if (strcmp(((function*)root->data)->name, "main") != 0) {
        fprintf(stderr, "%s:%d: error: root node is not the main function\n", __FILE__, __LINE__);
    }
    generate_template_asm(file, root);
    for (int i = 0; i < root->children->count; i++) {
        AST_node* node = ((AST_node**)root->children->data)[i];
        if (node->node_type == OPERATOR) {
            fprintf(file, "    movl $0, -%i(%%rbp)\n", ((varible*)((AST_node**)node->children->data)[0]->data)->stack_pos);
            compute_rvalue(file, ((AST_node**)node->children->data)[1]);
            fprintf(file, "    movl %%eax, -%i(%%rbp)\n", ((varible*)((AST_node**)node->children->data)[0]->data)->stack_pos);
            fprintf(file, "\n");
        } else if (node->node_type == KEY_WORD) {
            key_word* kw = (key_word*)node->data;
            switch (kw->key_word_type) {
            case RETURN: {
                // We need to do different things if we are returning from main or a user function
                if (strcmp(((function*)root->data)->name, "main") == 0) {
                    // Do syscall return
                    fprintf(file, "    movl -%i(%%rbp), %%eax\n", ((varible*)((AST_node**)node->children->data)[0]->data)->stack_pos);
                    fprintf(file, "    movl %%eax, %%edi\n");
                    fprintf(file, "    movl $60, %%eax\n");
                    fprintf(file, "    syscall\n");
                }
                else {
                    fprintf(stderr, "%s:%d: TODO: Function return is not handled\n", __FILE__, __LINE__);
                }
                break;
            }
            default: {
                fprintf(stderr, "%s:%d: TODO: Key word '%s' was not handled\n", __FILE__, __LINE__, kw->name);
                break;
            }
            }
        }
    }
}

int main()
{
    //char name[] = "./tests/if_statement/if_statement.c";
    char name[] = "./tests/test1/test1.c";
    file f;
    store_file(&f, name);
    for (int i = 0; i < f.size; i++) {
        printf("%c", f.data[i]);
    }
    int pos = 0;
    dynamic_array tokens;
    da_init(&tokens, token*);
        
    while (pos < f.size) {
        token* t = get_next_token(&f, &pos);
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

    AST_node* main_node = get_main_function(&tokens);
        
    int main_token_location = get_token_location(&tokens, main_node->token);
    int main_inputs_token_end = find_closing_paren(&tokens, main_token_location + 1);
    int start = main_inputs_token_end + 2;
    int end = find_closing_paren(&tokens, start - 1) - 1;
    
    main_node = create_body_AST_node(main_node, main_node, &tokens, start, end);
    generate_stack_posistions(main_node, 0);
    

    generate_graphviz_from_AST_node(main_node, "graph.gv");
    
    FILE* asm_file = fopen("output.asm", "w");
    create_asm_file(asm_file, main_node);
    fclose(asm_file);
    
    asm_file = fopen("output.asm", "r");
    char c;
    printf("\nAssembly file:\n\n");
    while ((c = getc(asm_file)) != EOF) {
        printf("%c", c);
    }

    return 0;
}
