
#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tokenizer.h"


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
} AST_node;

typedef struct {
    type* from_type;
    type* to_type;
} cast;

typedef struct {
    type* return_type;
    char* name;
    int frame_size;
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
AST_node* create_constant_node(AST_node* scope, dynamic_array* tokens, int start, int end);
AST_node* create_expression_node(AST_node* scope, dynamic_array* tokens, dynamic_array* types, int start, int end);
void create_varible_node(AST_node* scope, AST_node* node, dynamic_array* tokens, dynamic_array* types, token* t);
void create_return_node(AST_node* scope, AST_node* node, dynamic_array* tokens, dynamic_array* types, token* t);
int create_else_node(AST_node* scope, AST_node* node, dynamic_array* tokens, dynamic_array* types, token* t);
int create_if_node(AST_node* scope, AST_node* node, dynamic_array* tokens, dynamic_array* types, token* t);
int create_key_word_node(AST_node* scope, AST_node* node, dynamic_array* tokens, dynamic_array* types, token* t);
int match_tokens(AST_node* scope, AST_node* node, dynamic_array* tokens, dynamic_array* types, dynamic_array* token_stack);
void generate_AST(AST_node* scope, AST_node* node, dynamic_array* tokens, dynamic_array* types, int start, int end);
AST_node* create_function_node(dynamic_array* tokens, dynamic_array* types, int location);
void generate_functions(dynamic_array* functions, dynamic_array* types, dynamic_array* tokens);
int generate_stack_posistions(AST_node* scope, AST_node* node , int stack_size);

#endif // AST_H
