#ifndef ASM_GENERATOR_H
#define ASM_GENERATOR_H

#include <stdio.h>

#include "AST.h"


void generate_template_asm(FILE* file, AST_node* root);
char generate_asm_cast(FILE* file, AST_node* scope, AST_node* node);
char generate_rvalue_asm(FILE* file, AST_node* scope, AST_node* node);
void generate_if_asm(FILE* file, AST_node* scope, AST_node* node);
void generate_for_asm(FILE* file, AST_node* scope, AST_node* node);
char generate_asm_from_node(FILE* file, AST_node* scope, AST_node* node);
void create_asm_file(FILE* file, dynamic_array* functions);
char generate_asm_function_call(FILE* file, AST_node* scope, AST_node* node);

#endif // ASM_GENERATOR_H

