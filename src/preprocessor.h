#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "AST.h"
#include "tokenizer.h"
#include "graph.h"


typedef struct {
    token *name;
    dynamic_array *inputs; // dynamic_array of token pointers for inputs
    dynamic_array *output; // dynamic_array of token pointers for output
} define;

typedef enum {
    PRE_PROCESS_DEFINE,
    PRE_PROCESS_INCLUDE,
    PRE_PROCESS_IF,
    PRE_PROCESS_ELSE,
    PRE_PROCESS_ELIF,
    PRE_PROCESS_IFDEF,
    PRE_PROCESS_IFNDEF,
    PRE_PROCESS_ENDIF,
    PRE_PROCESS_UNDEF,
} PREPROCESSORS;

PREPROCESSORS get_preprocessor(dynamic_array *tokens, int location);
int get_end_of_define(dynamic_array *tokens, int start);
int get_end_of_line(dynamic_array *tokens, int start);
void replace_tokens_with_array(dynamic_array *tokens_dst, token *t, int start_range, int end_range, dynamic_array *tokens_src);
void replace_tokens(dynamic_array *tokens, token *t, int start_range, int end_range, int start, int end);
void remove_tokens(dynamic_array *tokens, int start, int end);
void create_define(dynamic_array *tokens, define *d, int start, int end);
void create_macro(dynamic_array *tokens, define *d, int macro_input_start, int macro_input_end, int end);
char *get_path(dynamic_array *paths, char *name);
int find_endif(dynamic_array *tokens, int start);
void do_preprocessor_ifndef(dynamic_array *tokens, dynamic_array *defines, dynamic_array *include_paths, int start, int end);
void do_preprocessor_ifdef(dynamic_array *tokens, dynamic_array *defines, dynamic_array *include_paths, int start, int end);
int evaluate_expression(dynamic_array *tokens, dynamic_array *defines, int start, int end);
void do_preprocessor_if(dynamic_array *tokens, dynamic_array *defines, dynamic_array *include_paths, int start, int end);
void do_preprocessor_include(dynamic_array *tokens, dynamic_array *defines, dynamic_array *include_paths, int start, int end);
void do_preprocessor_undefine(dynamic_array *tokens, dynamic_array *defines, int start, int end);
void replace_token_with_define(dynamic_array *tokens, define *d, int loc);
void do_preprocessor_define(dynamic_array *tokens, dynamic_array *defines, int start, int end);
void do_preprocessor(dynamic_array *tokens, dynamic_array *defines, dynamic_array *include_paths, int start, int key);
void preprocess_file(dynamic_array *tokens, dynamic_array *defines, dynamic_array *include_paths);

#endif // PREPROCESSOR_H
