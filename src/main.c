#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "AST.h"
#include "graph.h"
#include "assembly.h"

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
