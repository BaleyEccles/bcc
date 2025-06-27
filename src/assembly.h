#ifndef ASM_GENERATOR_H
#define ASM_GENERATOR_H

#include <stdio.h>

#include "AST.h"


void generate_template_asm(FILE* file, AST_node* root);
void generate_rvalue_asm(FILE* file, AST_node* scope, AST_node* node);
void generate_if_asm(FILE* file, AST_node* scope, AST_node* node);
void generate_for_asm(FILE* file, AST_node* scope, AST_node* node);
void generate_asm_from_node(FILE* file, AST_node* scope, AST_node* node);
void create_asm_file(FILE* file, AST_node* scope, AST_node* root);

#endif // ASM_GENERATOR_H

