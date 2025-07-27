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
    type_void->type_type = 0;
    da_append(ts, type_void, type*);
    
    type* type_bool = malloc(sizeof(type));
    type_bool->string = "bool";
    type_bool->size = 1;
    type_bool->ptr_count = 0;
    type_bool->type_type = 0;
    da_append(ts, type_bool, type*);

    type* type_char = malloc(sizeof(type));
    type_char->string = "char";
    type_char->size = 1;
    type_char->ptr_count = 0;
    type_char->type_type = 0;
    da_append(ts, type_char, type*);

    type* type_short = malloc(sizeof(type));
    type_short->string = "short";
    type_short->size = 2;
    type_short->ptr_count = 0;
    type_short->type_type = 0;
    da_append(ts, type_short, type*);

    type* type_int = malloc(sizeof(type));
    type_int->string = "int";
    type_int->size = 2;
    type_int->ptr_count = 0;
    type_int->type_type = 0;
    da_append(ts, type_int, type*);

    type* type_long = malloc(sizeof(type));
    type_long->string = "long";
    type_long->size = 4;
    type_long->ptr_count = 0;
    type_long->type_type = 0;
    da_append(ts, type_long, type*);

    type* type_long_long = malloc(sizeof(type));
    type_long_long->string = "long long";
    type_long_long->size = 8;
    type_long_long->ptr_count = 0;
    type_long_long->type_type = 0;
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
    free(flag);
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
    as_command[0] = '\0';
    strcat(as_command, as_default_command);
    strcat(as_command, as_inputs);
    free(as_default_command);

    char* ld_default_command = malloc(sizeof(char)*(strlen(output_file)*2 + 20));
    sprintf(ld_default_command, "ld -o %s %s.o ", output_file, output_file);
    char* ld_command = malloc(sizeof(char)*(strlen(ld_default_command) + strlen(ld_inputs) + 1));
    ld_command[0] = '\0';
    strcat(ld_command, ld_default_command);
    strcat(ld_command, ld_inputs);
    free(ld_default_command);
    
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

    // Default defines
    define* stdc_version = malloc(sizeof(define));
    token* stdc_version_token = malloc(sizeof(token));
    ALLOC_STR(stdc_version_token, "__STDC_VERSION__");
    stdc_version->name = stdc_version_token;
    
    stdc_version->inputs = NULL;
    
    dynamic_array* stdc_version_output = malloc(sizeof(dynamic_array));
    da_init(stdc_version_output, token*);
    
    token* stdc_version_token_output = malloc(sizeof(token));
    ALLOC_STR(stdc_version_token_output, "199901L");
    
    da_append(stdc_version_output, stdc_version_token_output, token*);    
    stdc_version->output = stdc_version_output;

    da_append(&defines, stdc_version, define*);


    define* file_offset_bits = malloc(sizeof(define));
    token* file_offset_bits_token = malloc(sizeof(token));
    ALLOC_STR(file_offset_bits_token, "_FILE_OFFSET_BITS");
    file_offset_bits->name = file_offset_bits_token;
    
    file_offset_bits->inputs = NULL;
    
    dynamic_array* file_offset_bits_output = malloc(sizeof(dynamic_array));
    da_init(file_offset_bits_output, token*);
    
    token* file_offset_bits_token_output = malloc(sizeof(token));
    ALLOC_STR(file_offset_bits_token_output, "64");
    
    da_append(file_offset_bits_output, file_offset_bits_token_output, token*);    
    file_offset_bits->output = file_offset_bits_output;

    da_append(&defines, file_offset_bits, define*);



    define* glibc_use = malloc(sizeof(define));
    token* glibc_use_token = malloc(sizeof(token));
    ALLOC_STR(glibc_use_token, "__GLIBC_USE");
    glibc_use->name = glibc_use_token;
    
    glibc_use->inputs = NULL;
    
    dynamic_array* glibc_use_output = malloc(sizeof(dynamic_array));
    da_init(glibc_use_output, token*);
    
    token* glibc_use_token_output = malloc(sizeof(token));
    ALLOC_STR(glibc_use_token_output, "0");
    
    da_append(glibc_use_output, glibc_use_token_output, token*);    
    glibc_use->output = glibc_use_output;

    da_append(&defines, glibc_use, define*);




    define* isoc23 = malloc(sizeof(define));
    token* isoc23_token = malloc(sizeof(token));
    ALLOC_STR(isoc23_token, "ISOC23");
    isoc23->name = isoc23_token;
    
    isoc23->inputs = NULL;
    
    dynamic_array* isoc23_output = malloc(sizeof(dynamic_array));
    da_init(isoc23_output, token*);
    
    token* isoc23_token_output = malloc(sizeof(token));
    ALLOC_STR(isoc23_token_output, "0");
    
    da_append(isoc23_output, isoc23_token_output, token*);    
    isoc23->output = isoc23_output;

    da_append(&defines, isoc23, define*);

    
    define* xopen_source = malloc(sizeof(define));
    token* xopen_source_token = malloc(sizeof(token));
    ALLOC_STR(xopen_source_token, "_XOPEN_SOURCE");
    xopen_source->name = xopen_source_token;
    
    xopen_source->inputs = NULL;
    
    dynamic_array* xopen_source_output = malloc(sizeof(dynamic_array));
    da_init(xopen_source_output, token*);
    
    token* xopen_source_token_output = malloc(sizeof(token));
    ALLOC_STR(xopen_source_token_output, "0");
    
    da_append(xopen_source_output, xopen_source_token_output, token*);    
    xopen_source->output = xopen_source_output;
    da_append(&defines, xopen_source, define*);



    define* x86_64 = malloc(sizeof(define));
    token* x86_64_token = malloc(sizeof(token));
    ALLOC_STR(x86_64_token, "__x86_64__");
    x86_64->name = x86_64_token;
    
    x86_64->inputs = NULL;
    
    dynamic_array* x86_64_output = malloc(sizeof(dynamic_array));
    da_init(x86_64_output, token*);
    
    token* x86_64_token_output = malloc(sizeof(token));
    ALLOC_STR(x86_64_token_output, "1");
    
    da_append(x86_64_output, x86_64_token_output, token*);    
    x86_64->output = x86_64_output;

    da_append(&defines, x86_64, define*);


    define* fortify_source = malloc(sizeof(define));
    token* fortify_source_token = malloc(sizeof(token));
    ALLOC_STR(fortify_source_token, "_FORTIFY_SOURCE");
    fortify_source->name = fortify_source_token;
    
    fortify_source->inputs = NULL;
    
    dynamic_array* fortify_source_output = malloc(sizeof(dynamic_array));
    da_init(fortify_source_output, token*);
    
    token* fortify_source_token_output = malloc(sizeof(token));
    ALLOC_STR(fortify_source_token_output, "0");
    
    da_append(fortify_source_output, fortify_source_token_output, token*);    
    fortify_source->output = fortify_source_output;

    da_append(&defines, fortify_source, define*);

    // End Default defines

    dynamic_array include_paths;
    da_init(&include_paths, char*);
    
    da_append(&include_paths, "./", char*);
    da_append(&include_paths, "/usr/local/musl/include", char*);
    //da_append(&include_paths, "/usr/include", char*);
    //da_append(&include_paths, "/usr/include/linux", char*);
    //da_append(&include_paths, "/usr/lib/gcc/x86_64-pc-linux-gnu/15.1.1/include", char*);

    remove_comments(&tokens);
    preprocess_file(&tokens, &defines, &include_paths);
    clean_tokens(&tokens);
    

    generate_types(&ctx);

    for (int i = 0; i < tokens.count; i++) {
        get_token_type(&types, &tokens, ((token**)tokens.data)[i]);
        //printf("token with index %i at %i: %s and type %i\n", i,  ((token**)tokens.data)[i]->pos_in_file, ((token**)tokens.data)[i]->data, ((token**)tokens.data)[i]->type);
        //printf("%s ", ((token**)tokens.data)[i]->data);
    }

    // Generate AST
    
    generate_functions(&ctx);

    for (int i = 0; i < functions.count; i++) {
        AST_node* f = ((AST_node**)functions.data)[i];
        char* graph_name = malloc(sizeof(char)*(10 +strlen(((function*)f->data)->name)));

        
        sprintf(graph_name, "graph_%s.gv", ((function*)f->data)->name);
        //printf("f: %s\n", graph_name);
        generate_graphviz_from_AST_node(f, graph_name);
        free(graph_name);
    }

    for (int i = 0; i < functions.count; i++) {
        AST_node* f = ((AST_node**)functions.data)[i];
        f->types = &types;
        generate_stack_posistions(&ctx, f, f, 0);
        print_vars(f);
    }

    

    char* asm_file_name = malloc(sizeof(char)*(strlen(output_file) + 5));
    sprintf(asm_file_name, "%s.asm", output_file);
    
    FILE* asm_file = fopen(asm_file_name, "w");
    create_asm_file(asm_file, &functions);

    fclose(asm_file);
    
    asm_file = fopen(asm_file_name, "r");
    free(asm_file_name);

    printf("INFO: Running: %s\n", as_command);
    system(as_command);
    free(as_command);
    printf("INFO: Running: %s\n", ld_command);
    system(ld_command);
    free(ld_command);

    for (int i = 0; i < types.count; i++) {
        type* t = ((type**)types.data)[i];
        free(t);
    }
    free(types.data);

    for (int i = 0; i < tokens.count; i++) {
        token* t = ((token**)tokens.data)[i];
        free(t->data);
        free(t);
    }
    free(tokens.data);
    free(functions.data);
    free(include_paths.data);

    for (int i = 0; i < defines.count; i++) {
        define* d = ((define**)defines.data)[i];
        if (d != NULL) {
            for (int j = 0; j < d->output->count; j++) {
                token* t = ((token**)d->output->data)[j];
                free(t->data);
                free(t);
            }

            da_destroy(d->output, token*);
            if (d->inputs != NULL) {
                for (int j = 0; j < d->inputs->count; j++) {
                    token* t = ((token**)d->inputs->data)[j];
                    free(t->data);
                    free(t);
                }
                da_destroy(d->inputs, token*);
            }
            free(d->name->data);
            free(d->name);
            free(d);
        }
    }
    free(defines.data);
    
   
    return 0;
    
}
