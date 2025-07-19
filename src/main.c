#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "AST.h"
#include "graph.h"
#include "assembly.h"
#include "preprocessor.h"

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
    type_char->size = 1;
    type_char->ptr_count = 0;
    da_append(ts, type_char, type*);

    type* type_short = malloc(sizeof(type));
    type_short->string = "short";
    type_short->size = 2;
    type_short->ptr_count = 0;
    da_append(ts, type_short, type*);

    type* type_int = malloc(sizeof(type));
    type_int->string = "int";
    type_int->size = 2;
    type_int->ptr_count = 0;
    da_append(ts, type_int, type*);

    type* type_long = malloc(sizeof(type));
    type_long->string = "long";
    type_long->size = 4;
    type_long->ptr_count = 0;
    da_append(ts, type_long, type*);

    type* type_long_long = malloc(sizeof(type));
    type_long_long->string = "long long";
    type_long_long->size = 8;
    type_long_long->ptr_count = 0;
    da_append(ts, type_long_long, type*);

    // TODO: Floats/doubles
}


char* parse_inputs(char* argv)
{
    // argv Should look like -A=-g -o blah blah blah
    int low_range = 3;
    int len = strlen(argv) - low_range;
    char* inputs = malloc(sizeof(char)*(len));
    inputs = strncpy(inputs, argv + low_range, len);
    inputs[len] = '\0';
    return inputs;
}

int main(int argc, char *argv[])
{

    char* flag = malloc(sizeof(char)*3);
    char* as_inputs = NULL;
    char* ld_inputs = NULL;
    char* input_file_name = NULL;
    char* output_file = NULL;
    for (int i = 1; i < argc; i++) {
        //printf("Argument %d: %s\n", i, argv[i]);
        flag[0] = argv[i][0];
        flag[1] = argv[i][1];
        flag[2] = '\0';
        
        if (strcmp(flag, "-A") == 0) {
            as_inputs = parse_inputs(argv[i]);
        } else if (strcmp(flag, "-L") == 0) {
            ld_inputs = parse_inputs(argv[i]);
        } else if (strcmp(flag, "-o") == 0) {
            output_file = argv[i + 1];
            i++;
        } else {
            input_file_name = argv[i];
        }
    }
    if (output_file == NULL) {
        output_file = "output";
    }
    if (as_inputs == NULL) {
        as_inputs = "";
    }
    if (ld_inputs == NULL) {
        ld_inputs = "";
    }
    if (input_file_name == NULL) {
        fprintf(stderr, "%s:%d: error: No input file supplied\n", __FILE__, __LINE__);
        input_file_name = "./tests/if_statement/if_statement.c";
    }


    char* as_default_command = malloc(sizeof(char)*(strlen(output_file)*2 + 20));
    sprintf(as_default_command, "as -g -o %s.o %s.asm ", output_file, output_file);

    char* as_command = malloc(sizeof(char)*(strlen(as_default_command) + strlen(as_inputs) + 1));
    strcat(as_command, as_default_command);
    strcat(as_command, as_inputs);

    char* ld_default_command = malloc(sizeof(char)*(strlen(output_file)*2 + 20));
    sprintf(ld_default_command, "ld -o %s %s.o ", output_file, output_file);
    char* ld_command = malloc(sizeof(char)*(strlen(ld_default_command) + strlen(ld_inputs) + 1));
    strcat(ld_command, ld_default_command);
    strcat(ld_command, ld_inputs);
    
    FILE* input_file = fopen(input_file_name, "r");
    
    fseek(input_file, 0L, SEEK_END);
    int input_file_size = ftell(input_file);
    rewind(input_file);


    dynamic_array tokens;
    da_init(&tokens, token*);
    dynamic_array types;
    da_init(&types, type*);
    dynamic_array functions;
    da_init(&functions, AST_node*);
    context ctx = {&tokens, &types, &functions};

    int pos = 0;
    while (pos < input_file_size) {
        token* t = get_next_token(input_file, &pos);
        da_append(&tokens, t, token*);
    }
    untabbify_tokens(&tokens);

    // Generate default types
    generate_default_types(&types);

    
    for (int i = 0; i < tokens.count; i++) {
        get_token_type(&types, &tokens, ((token**)tokens.data)[i]);
        //printf("token with index %i at %i: %s and type %i\n", i,  ((token**)tokens.data)[i]->pos_in_file, ((token**)tokens.data)[i]->data, ((token**)tokens.data)[i]->type);
    }

    
    // Pre proccess
    dynamic_array defines;
    da_init(&defines, define*);

    dynamic_array include_paths;
    da_init(&include_paths, char*);
    da_append(&include_paths, "/usr/include", char*);

    remove_comments(&tokens);
    preprocess_file(&tokens, &defines, &include_paths);
    clean_tokens(&tokens);
    



    for (int i = 0; i < tokens.count; i++) {
        get_token_type(&types, &tokens, ((token**)tokens.data)[i]);
        //printf("token with index %i at %i: %s and type %i\n", i,  ((token**)tokens.data)[i]->pos_in_file, ((token**)tokens.data)[i]->data, ((token**)tokens.data)[i]->type);
        //printf("%s ", ((token**)tokens.data)[i]->data);
    }

    // Generate AST
    generate_functions(&ctx);

    for (int i = 0; i < functions.count; i++) {
        AST_node* f = ((AST_node**)functions.data)[i];
        char* graph_name = malloc(sizeof(char)*9 + sizeof(char)*strlen(((function*)f->data)->name));
        sprintf(graph_name, "graph_%s.gv", ((function*)f->data)->name);
        generate_graphviz_from_AST_node(f, graph_name);
    }

    for (int i = 0; i < functions.count; i++) {
        AST_node* f = ((AST_node**)functions.data)[i];
        f->types = &types;
        generate_stack_posistions(f, f, 0);

    }

    char* asm_file_name = malloc(sizeof(char)*(strlen(output_file) + 5));
    sprintf(asm_file_name, "%s.asm", output_file);
    
    FILE* asm_file = fopen(asm_file_name, "w");
    create_asm_file(asm_file, &functions);

    fclose(asm_file);
    
    asm_file = fopen(asm_file_name, "r");

    printf("INFO: Running: %s\n", as_command);
    system(as_command);
    printf("INFO: Running: %s\n", ld_command);
    system(ld_command);
   
    return 0;
    
}
