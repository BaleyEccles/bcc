
#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tokenizer.h"


// Enum definitions
typedef enum {
    FUNCTION,
    VARIBLE,
    CONSTANT,
    OPERATOR,
    KEY_WORD,
} NODE_TYPE;

// Structure definitions
typedef struct AST_node {
    dynamic_array* children;
    NODE_TYPE node_type;
    token* token;
    void* data;
} AST_node;

typedef struct {
    type* type;
    char* name;
} function_input;

typedef struct {
    type* return_type;
    char* name;
    dynamic_array* inputs;
} function;

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
AST_node* get_node_from_pos_recursive(AST_node* node, int pos);
AST_node* get_node_from_pos(AST_node* node, int pos);
AST_node* get_varible_node_from_name_recursive(AST_node* node, char* name);
AST_node* get_varible_node_from_name(AST_node* node, char* name);
bool is_varible_defined(AST_node* node, char* str);
int find_opening_paren(dynamic_array* tokens, int starting_token_location);
int find_closing_paren(dynamic_array* tokens, int starting_token_location);
AST_node* get_main_function(dynamic_array* tokens);
AST_node* create_single_rvalue_node(AST_node* scope, dynamic_array* tokens, int start_location, int end_location);
AST_node* create_expression_AST_node(AST_node* scope, dynamic_array* tokens, int start_location, int end_location);
int generate_stack_posistions(AST_node* scope, int stack_size);
AST_node* create_body_AST_node(AST_node* scope, AST_node* node, dynamic_array* tokens, int start, int end);

#endif // AST_H
