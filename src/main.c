#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "AST.h"
#include "graph.h"
#include "assembly.h"

// https://en.wikipedia.org/wiki/C_data_typese
void generate_default_types(dynamic_array* ts)
{
    type* type_void = malloc(sizeof(type));
    type_void->string = "void";
    type_void->size = 0;
    type_void->ptr_count = 0;
    da_append(ts, type_void, type*);
    
    type* type_bool = malloc(sizeof(type));
    type_bool->string = "bool";
    type_bool->size = 1;
    type_bool->ptr_count = 0;
    da_append(ts, type_bool, type*);

    type* type_char = malloc(sizeof(type));
    type_char->string = "char";
    type_char->size = 8;
    type_char->ptr_count = 0;
    da_append(ts, type_char, type*);

    type* type_short = malloc(sizeof(type));
    type_short->string = "short";
    type_short->size = 16;
    type_short->ptr_count = 0;
    da_append(ts, type_short, type*);

    type* type_int = malloc(sizeof(type));
    type_int->string = "int";
    type_int->size = 16;
    type_int->ptr_count = 0;
    da_append(ts, type_int, type*);

    type* type_long = malloc(sizeof(type));
    type_long->string = "long";
    type_long->size = 32;
    type_long->ptr_count = 0;
    da_append(ts, type_long, type*);

    type* type_long_long = malloc(sizeof(type));
    type_long_long->string = "long long";
    type_long_long->size = 64;
    type_long_long->ptr_count = 0;
    da_append(ts, type_long_long, type*);

    // TODO: Floats/doubles
}

int main()
{
    char input_name[] = "./tests/types/types.c";
    //char input_name[] = "./tests/fibonacci/fibonacci.c";
    //char input_name[] = "./tests/function/function.c";
    //char input_name[] = "./tests/while_loop/while_loop.c";
    //char input_name[] = "./tests/for_loop/for_loop.c";
    //char input_name[] = "./tests/if_statement/if_statement.c";
    //char input_name[] = "./tests/test1/test1.c";
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
    dynamic_array types;
    da_init(&types, type);
    generate_default_types(&types);
    
    clean_tokens(&tokens);
    for (int i = 0; i < tokens.count; i++) {
        get_token_type(&types, &tokens, ((token**)tokens.data)[i]);
        printf("token with index %i at %i: %s and type %i\n", i,  ((token**)tokens.data)[i]->pos_in_file, ((token**)tokens.data)[i]->data, ((token**)tokens.data)[i]->type);
        //printf("%s ", ((token**)tokens.data)[i]->data);
    }

    
    // Generate AST
    
    dynamic_array functions;
    da_init(&functions, AST_node*);
    
    generate_functions(&functions, &tokens, &types);

    for (int i = 0; i < functions.count; i++) {
        AST_node* f = ((AST_node**)functions.data)[i];
        char* graph_name = malloc(sizeof(char)*9 + sizeof(char)*strlen(((function*)f->data)->name));
        sprintf(graph_name, "graph_%s.gv", ((function*)f->data)->name);
        generate_graphviz_from_AST_node(f, graph_name);
    }

    for (int i = 0; i < functions.count; i++) {
        AST_node* f = ((AST_node**)functions.data)[i];
        generate_stack_posistions(f, f, 0);
    }
    
    FILE* asm_file = fopen("output.asm", "w");
    create_asm_file(asm_file, &functions);

    fclose(asm_file);
    
    asm_file = fopen("output.asm", "r");
    printf("\nAssembly file:\n\n");
    while ((c = getc(asm_file)) != EOF) {
        printf("%c", c);
    }
    
    return 0;
}
