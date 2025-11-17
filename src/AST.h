
#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tokenizer.h"



typedef struct {
    dynamic_array* tokens;
    dynamic_array* types;
    dynamic_array* functions;
} context;


// Enum definitions
typedef enum {
    NONE = 0,
    FUNCTION,
    FUNCTION_INPUT,
    FUNCTION_CALL,
    VARIBLE,
    CONSTANT,
    OPERATOR,
    KEY_WORD,
    CAST,
    ACCESS,
} NODE_TYPE;

// Structure definitions
typedef struct AST_node {
    dynamic_array* children;
    NODE_TYPE node_type;
    token* token;
    void* data;

    dynamic_array* types;
} AST_node;

typedef struct {
    type* from_type;
    type* to_type;
} cast;

typedef struct {
    type* return_type;
    char* name;
    int frame_size;
    int str_count;
} function;

typedef struct {
    type* return_type;
    char* name;
} function_call;

typedef struct {
    type* type;
    char* value;
} constant;

typedef struct {
    type* type;
    char* name;
    int stack_pos;
} varible;

typedef struct {
    char* name;
    TOKEN_TYPE type;
} operator;

typedef struct {
    char* name;
    TOKEN_TYPE key_word_type;
    void* data; // I am unsure if this data will be needed, but it will be here just in case
} key_word;

// Function prototypes

void init_AST_node(AST_node* node);
AST_node* get_node_from_pos(AST_node* node, int pos);
AST_node* get_node_from_name(AST_node* node, char* name);
bool is_varible_defined(AST_node* node, char* str);
int get_opening_paren_location(dynamic_array* tokens, int starting_token_location);
int get_closing_paren_location(dynamic_array* tokens, int starting_token_location);
AST_node* create_constant_node(AST_node* scope, context* ctx, int start, int end);
AST_node* create_expression_node(AST_node* scope, context* ctx, int start, int end);
void create_varible_node(AST_node* scope, AST_node* node, context* ctx, token* t);
void create_return_node(AST_node* scope, AST_node* node, context* ctx, token* t);
int create_else_node(AST_node* scope, AST_node* node, context* ctx, token* t);
int create_if_node(AST_node* scope, AST_node* node, context* ctx, token* t);
int create_key_word_node(AST_node* scope, AST_node* node, context* ctx, token* t);
int match_tokens(AST_node* scope, AST_node* node, context* ctx, dynamic_array* token_stack);
void generate_AST(AST_node* scope, AST_node* node, context* ctx, int start, int end);
AST_node* create_function_node(context* ctx, int location);
void generate_types(context* ctx);
void generate_functions(context* ctx);
int generate_stack_posistions(context* ctx, AST_node* scope, AST_node* node , int stack_size);
int evaluate_node(AST_node* n);
type* get_type_from_node(dynamic_array* types, AST_node* node);

void print_vars(AST_node* node);

extern void add_nodes_to_graphviz_file(AST_node* node, FILE* f, int depth);
extern void generate_graphviz_from_AST_node(AST_node* node, char* file_name);

#endif // AST_H
