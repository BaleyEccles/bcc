#include "assembly.h"

typedef struct {
    char* name;
    int size;
} name_size_mapping;

static const name_size_mapping register_mapping[] = {
    {"%%rax", 8}, {"%%eax",  4}, {"%%ax",   2}, {"%%al",   1},
    {"%%rbx", 8}, {"%%ebx",  4}, {"%%bx",   2}, {"%%bl",   1},
    {"%%rcx", 8}, {"%%ecx",  4}, {"%%cx",   2}, {"%%cl",   1},
    {"%%rdx", 8}, {"%%edx",  4}, {"%%dx",   2}, {"%%dl",   1},
    {"%%rsi", 8}, {"%%esi",  4}, {"%%si",   2}, {"%%sil",  1},
    {"%%rdi", 8}, {"%%edi",  4}, {"%%di",   2}, {"%%dil",  1},
    {"%%rbp", 8}, {"%%ebp",  4}, {"%%bp",   2}, {"%%bpl",  1},
    {"%%rsp", 8}, {"%%esp",  4}, {"%%sp",   2}, {"%%spl",  1},
    {"%%r8",  8}, {"%%r8d",  4}, {"%%r8w",  2}, {"%%r8b",  1},
    {"%%r9",  8}, {"%%r9d",  4}, {"%%r9w",  2}, {"%%r9b",  1},
    {"%%r10", 8}, {"%%r10d", 4}, {"%%r10w", 2}, {"%%r10b", 1},
    {"%%r11", 8}, {"%%r11d", 4}, {"%%r11w", 2}, {"%%r11b", 1},
    {"%%r12", 8}, {"%%r12d", 4}, {"%%r12w", 2}, {"%%r12b", 1},
    {"%%r13", 8}, {"%%r13d", 4}, {"%%r13w", 2}, {"%%r13b", 1},
    {"%%r14", 8}, {"%%r14d", 4}, {"%%r14w", 2}, {"%%r14b", 1},
    {"%%r15", 8}, {"%%r15d", 4}, {"%%r15w", 2}, {"%%r15b", 1},
};

static const name_size_mapping size_mapping[] = {
    {"q", 8}, {"l", 4}, {"w", 2}, {"b", 1},
};

void generate_rvalue_asm(FILE* file, AST_node* scope, AST_node* node)
{
    if (node->node_type == OPERATOR) {
        AST_node* child1 = ((AST_node**)node->children->data)[0];
        AST_node* child2 = ((AST_node**)node->children->data)[1];
        
        
        TOKEN_TYPE t = ((operator*)node->data)->type;
        switch (t) {
        case PLUS: {
            fprintf(file, "# %s + %s\n", child1->token->data, child2->token->data);
            generate_rvalue_asm(file, scope, child2);
            if (child1->node_type == CONSTANT) {
                fprintf(file, "    addl $%s, %%eax\n", ((constant*)child1->data)->value);
            } else if (child1->node_type == FUNCTION_CALL) {
                fprintf(file, "    push %%rax\n");
                generate_asm_from_node(file, scope, child1);
                fprintf(file, "    movl %%eax, %%edi\n");
                fprintf(file, "    pop %%rax\n");
                fprintf(file, "    addl %%edi, %%eax\n");
            } else {
                fprintf(file, "    addl -%i(%%rbp), %%eax\n", ((varible*)child1->data)->stack_pos);
            }
            break;
        }
        case PLUS_EQUALS: {
            fprintf(file, "# %s += %s\n", child1->token->data, child2->token->data);
            generate_asm_from_node(file, scope, child2);
            fprintf(file, "    addl %%eax, -%i(%%rbp)\n", ((varible*)child1->data)->stack_pos);
            break;
        }
        case MINUS: {
            fprintf(file, "# %s - %s\n", child1->token->data, child2->token->data);
            generate_rvalue_asm(file, scope, child1);
            if (child2->node_type == CONSTANT) {
                fprintf(file, "    subl $%s, %%eax\n", ((constant*)child2->data)->value);
            } else if (child2->node_type == FUNCTION_CALL) {
                fprintf(file, "    push %%rax\n");
                generate_asm_from_node(file, scope, child2);
                fprintf(file, "    movl %%eax, %%edi\n");
                fprintf(file, "    pop %%rax\n");
                fprintf(file, "    subl %%edi, %%eax\n");
                
            } else {
                fprintf(file, "    subl -%i(%%rbp), %%eax\n", ((varible*)child2->data)->stack_pos);
            }
            break;
        }
        case POST_INCREMENT: {
            if (child1->node_type == CONSTANT) {
                fprintf(file, "    movl $%s, %%eax\n", ((constant*)child1->data)->value);
                fprintf(file, "    addl $1, %%eax\n");
            } else {
                fprintf(file, "    movl -%i(%%rbp), %%eax\n", ((varible*)child1->data)->stack_pos);
                fprintf(file, "    addl $1, %%eax\n");
                fprintf(file, "    movl %%eax, -%i(%%rbp)\n", ((varible*)child1->data)->stack_pos);
            }
            break;
        }
        case LOGICAL_EQUALS: {
            fprintf(file, "# %s == %s\n", child1->token->data, child2->token->data);
            generate_rvalue_asm(file, scope, child1);
            fprintf(file, "    movl %%eax, %%esi\n");
            generate_rvalue_asm(file, scope, child2);
            fprintf(file, "    movl %%eax, %%edi\n");
            fprintf(file, "    cmpl %%edi, %%esi\n");
            fprintf(file, "    je ");
            break;
        }
        case GREATER_THAN: {
            generate_rvalue_asm(file, scope, child1);
            fprintf(file, "    movl %%eax, %%esi\n");
            generate_rvalue_asm(file, scope, child2);
            fprintf(file, "    movl %%eax, %%edi\n");
            fprintf(file, "    cmpl %%edi, %%esi\n");
            fprintf(file, "    jg ");
            break;
        }
        case GREATER_THAN_OR_EQUALS: {
            generate_rvalue_asm(file, scope, child1);
            fprintf(file, "    movl %%eax, %%esi\n");
            generate_rvalue_asm(file, scope, child2);
            fprintf(file, "    movl %%eax, %%edi\n");
            fprintf(file, "    cmpl %%edi, %%esi\n");
            fprintf(file, "    jge ");
            break;
        }
        case LESS_THAN: {
            generate_rvalue_asm(file, scope, child1);
            fprintf(file, "    movl %%eax, %%esi\n");
            generate_rvalue_asm(file, scope, child2);
            fprintf(file, "    movl %%eax, %%edi\n");
            fprintf(file, "    cmpl %%edi, %%esi\n");
            fprintf(file, "    jl ");
            break;
        }
        case LESS_THAN_OR_EQUALS: {
            generate_rvalue_asm(file, scope, child1);
            fprintf(file, "    movl %%eax, %%esi\n");
            generate_rvalue_asm(file, scope, child2);
            fprintf(file, "    movl %%eax, %%edi\n");
            fprintf(file, "    cmpl %%edi, %%esi\n");
            fprintf(file, "    jle ");
            break;
        }
        case LOGICAL_AND: {
            generate_rvalue_asm(file, scope, child1);
            fprintf(file, "AND_%i\n", node->token->pos_in_file);
            fprintf(file, "    jmp AND_false_%i\n", node->token->pos_in_file);
            
            fprintf(file, "AND_%i:\n", node->token->pos_in_file);
            generate_rvalue_asm(file, scope, child2);
            fprintf(file, "AND_true_%i\n", node->token->pos_in_file);
            fprintf(file, "    jmp AND_false_%i\n", node->token->pos_in_file);
            
            fprintf(file, "AND_false_%i:\n", node->token->pos_in_file);
            fprintf(file, "    movl $0, %%eax\n");
            fprintf(file, "    jmp AND_end_%i\n", node->token->pos_in_file);
            fprintf(file, "AND_true_%i:\n", node->token->pos_in_file);
            fprintf(file, "    movl $1, %%eax\n");
            fprintf(file, "    jmp AND_end_%i\n", node->token->pos_in_file);
            
            fprintf(file, "AND_end_%i:\n", node->token->pos_in_file);
            fprintf(file, "    cmpl $1, %%eax\n");
            fprintf(file, "    je ");
            
            break;
        }
        default: {
        }
            fprintf(stderr, "%s:%d: TODO: Operator %s %i was not handled\n", __FILE__, __LINE__, node->token->data, node->token->pos_in_file);
            fprintf(file  , "%s:%d: TODO: Operator %s %i was not handled\n", __FILE__, __LINE__, node->token->data, node->token->pos_in_file);
        }
        
        
    }
    if (node->node_type == FUNCTION_CALL) {
        generate_asm_function_call(file, scope, node);
    }
    if (node->node_type == VARIBLE) {
        fprintf(file, "    movl -%i(%%rbp), %%eax\n", ((varible*)node->data)->stack_pos);
    }
    if (node->node_type == CONSTANT) {
        if (type_is_string(((constant*)node->data)->type)) {
            fprintf(file, "    leaq .STR_%s_%i(%%rip), %%rax\n", ((function*)scope->data)->name, ((function*)scope->data)->str_count);
        } else {
            fprintf(file, "    movl $%s, %%eax\n", ((constant*)node->data)->value);
        }
    }
}



void generate_asm_from_node(FILE* file, AST_node* scope, AST_node* node);

    
void generate_if_asm(FILE* file, AST_node* scope, AST_node* node)
{
    fprintf(file, "# if \n");
    if (node->children->count == 3) {
        fprintf(file, "# else \n");
        AST_node* else_node = ((AST_node**)node->children->data)[2];
        
        generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
        fprintf(file, "%s_%i\n", node->token->data, node->token->pos_in_file);
        fprintf(file, "    jmp %s_%i\n", else_node->token->data, else_node->token->pos_in_file);
 
        fprintf(file, "%s_%i:\n", node->token->data, node->token->pos_in_file);
        generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
        fprintf(file, "    jmp end_%s_%i\n", node->token->data, node->token->pos_in_file);
        
        fprintf(file, "%s_%i:\n", else_node->token->data, else_node->token->pos_in_file);
        generate_asm_from_node(file, scope, else_node);
    } else if (node->children->count == 2) {

        generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
        fprintf(file, "%s_%i\n", node->token->data, node->token->pos_in_file);
        fprintf(file, "    jmp end_%s_%i\n", node->token->data, node->token->pos_in_file);
        
        fprintf(file, "%s_%i:\n", node->token->data, node->token->pos_in_file);
        generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
        fprintf(file, "    jmp end_%s_%i\n", node->token->data, node->token->pos_in_file);
    }
    fprintf(file, "end_%s_%i:\n", node->token->data, node->token->pos_in_file);
    
}

void generate_for_asm(FILE* file, AST_node* scope, AST_node* node)
{
    generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[0]);
    fprintf(file, ".start_%s_%i:\n", node->token->data, node->token->pos_in_file);
    generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[3]);
    generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[2]);
    generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[1]);
    fprintf(file, ".start_%s_%i\n", node->token->data, node->token->pos_in_file);
    fprintf(file, "    jmp .end_%s_%i\n", node->token->data, node->token->pos_in_file);
    fprintf(file, ".end_%s_%i:\n", node->token->data, node->token->pos_in_file);
}

void generate_while_asm(FILE* file, AST_node* scope, AST_node* node)
{
    fprintf(file, ".start_%s_%i:\n", node->token->data, node->token->pos_in_file);
    generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
    generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
    fprintf(file, ".start_%s_%i\n", node->token->data, node->token->pos_in_file);
    fprintf(file, "    jmp .end_%s_%i\n", node->token->data, node->token->pos_in_file);
    fprintf(file, ".end_%s_%i:\n", node->token->data, node->token->pos_in_file);
}


// https://en.wikipedia.org/wiki/X86_calling_conventions#x86-64_calling_conventions
#define CALLING_REGISTERS_COUNT (6)
static const char* registers[] = {
    "%edi", "%esi", "%edx", "%ecx", "%e8", "%e9"
};

void generate_asm_function_call(FILE* file, AST_node* scope, AST_node* node)
{
    fprintf(file, "# Calling %s\n", node->token->data);
    for (int i = 0; i < node->children->count; i++) {
        AST_node* child = ((AST_node**)node->children->data)[i];
        generate_rvalue_asm(file, scope, child);
        if (i < CALLING_REGISTERS_COUNT) {
            fprintf(file, "    movl %%eax, %s\n", registers[i]);
            
        } else {
            fprintf(stderr, "%s:%d: TODO: Calling function with more than 6 inputs is not done yet\n \n", __FILE__, __LINE__);
        }
    }
    fprintf(file, "    call %s\n", ((function_call*)node->data)->name);
}

void generate_asm_cast(FILE* file, AST_node* scope, AST_node* node)
{
    generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[0]);
    
    int from_size = ((cast*)node->data)->from_type->size;
    int to_size = ((cast*)node->data)->to_type->size;

    if ((from_size == 1 && to_size == 1) ||
        (from_size == 2 && to_size == 2) ||
        (from_size == 4 && to_size == 4) ||
        (from_size == 8 && to_size == 8)) {
        return;
    } else if (from_size == 1 && to_size == 2) {
        fprintf(file, "movsb %%al, %%ax\n");
    } else if (from_size == 1 && to_size == 4) {
        fprintf(file, "movsbl %%al, %%eax\n");
    } else if (from_size == 1 && to_size == 8) {
        fprintf(file, "movsbq %%al, %%rax\n");
    } else if (from_size == 2 && to_size == 1) {
        fprintf(file, "movsw %%ax, %%al\n");
    } else if (from_size == 2 && to_size == 4) {
        fprintf(file, "movsw %%ax, %%eax\n");
    } else if (from_size == 2 && to_size == 8) {
        fprintf(file, "movsw %%ax, %%rax\n");
    } else if (from_size == 4 && to_size == 1) {
        fprintf(file, "movb %%eax, %%al\n");
    } else if (from_size == 4 && to_size == 2) {
        fprintf(file, "movw %%eax, %%ax\n");
    } else if (from_size == 4 && to_size == 8) {
        fprintf(file, "movsl %%eax, %%rax\n");
    } else if (from_size == 8 && to_size == 1) {
        fprintf(file, "movb %%rax, %%al\n");
    } else if (from_size == 8 && to_size == 2) {
        fprintf(file, "movw %%rax, %%ax\n");
    } else if (from_size == 8 && to_size == 4) {
        fprintf(file, "movl %%rax, %%eax\n");
    } else {
        fprintf(stderr, "%s:%d: error: Unable to cast from %i to %i, token %s, %i\n", __FILE__, __LINE__, from_size, to_size, node->token->data, node->token->pos_in_file);
        *(int*)0 = 0;
    }
}


void generate_asm_access(FILE* file, AST_node* scope, AST_node* node)
{
    generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
    // Thing in squre brackets should be in eax
    fprintf(file, "    movzl %%eax, %%rax\n");
    
    AST_node* var_node = ((AST_node**)node->children->data)[0];
    fprintf(file, "    addq -%i(%%rbp), %%rax\n", ((varible*)var_node->data)->stack_pos);
    fprintf(file, "    movzbl (%%rax), %%eax\n");
}

void generate_asm_from_node(FILE* file, AST_node* scope, AST_node* node)
{
    if (node->node_type == OPERATOR && ((operator*)node->data)->type == EQUALS) {
        generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[1]);
        fprintf(file, "    movl %%eax, -%i(%%rbp)\n", ((varible*)((AST_node**)node->children->data)[0]->data)->stack_pos);
    } else if (node->node_type == FUNCTION_CALL) {
        generate_asm_function_call(file, scope, node);
        
    } else if (node->node_type == KEY_WORD) {
        key_word* kw = (key_word*)node->data;
        switch (kw->key_word_type) {
        case RETURN: {
            // We need to do different things if we are returning from main or a user function
            if (scope->node_type == FUNCTION) {
                if (strcmp(((function*)scope->data)->name, "main") == 0) {
                    // Do syscall return
                    generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
                    fprintf(file, "# return\n");
                    fprintf(file, "    movl %%eax, %%edi\n");
                    fprintf(file, "    movl $60, %%eax\n");
                    fprintf(file, "    syscall\n");
                }
                else {
                    // Do function return
                    generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
                    fprintf(file, "# return\n");
                    fprintf(file, "    jmp end_%s\n",((function*)scope->data)->name);
                }
            }
            
            break;
        }
        case IF: {
            generate_if_asm(file, scope, node);
            break;
        }
        case ELSE: {
            generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[0]);
            break;
        }
        case FOR: {
            generate_for_asm(file, scope, node);
            break;
        }
        case WHILE: {
            generate_while_asm(file, scope, node);
            break;
        }
        default: {
            fprintf(stderr, "%s:%d: TODO: Key word '%s' was not handled\n", __FILE__, __LINE__, kw->name);
            break;
        }
        }
    } else if (node->node_type == OPERATOR) {
        generate_rvalue_asm(file, scope, node);
    } else if (node->node_type == ACCESS) {
        generate_asm_access(file, scope, node);
    } else if (node->node_type == CAST) {
        generate_asm_cast(file, scope, node);
    } else if (node->node_type == FUNCTION_INPUT) {
    } else {
        fprintf(stderr, "%s:%d: TODO: AST node %s, %i was not handled\n", __FILE__, __LINE__, node->token->data, node->token->pos_in_file);
    }

}

void create_function_strings(FILE* file, AST_node* root, AST_node* node)
{
    for (int i = 0; i < node->children->count; i++) {
        create_function_strings(file, root, ((AST_node**)node->children->data)[i]);
    }
    if (node->node_type == CONSTANT) {
        constant* c = (constant*)node->data;
        if (type_is_string(c->type)) {
            ((function*)root->data)->str_count++;
            fprintf(file, ".STR_%s_%i:\n", ((function*)root->data)->name, ((function*)root->data)->str_count);
            fprintf(file, "    .string %s\n", c->value);
        }
    }
}


void create_asm_function(FILE* file, AST_node* root)
{
    char* function_name;
    if (strcmp(((function*)root->data)->name, "main") == 0) {
        function_name = "_start";
    } else {
        function_name = ((function*)root->data)->name;
    }

    create_function_strings(file, root, root);
    fprintf(file, "    .text\n");
    fprintf(file, "    .globl %s\n", function_name);
    fprintf(file, "    .type  %s, @function\n", function_name);
    fprintf(file, "%s:\n", function_name);
    fprintf(file, "    pushq %%rbp\n");
    fprintf(file, "    movq %%rsp, %%rbp\n");
    fprintf(file, "    subq $%i, %%rsp\n", ((function*)root->data)->frame_size);

    AST_node* function_inputs = ((AST_node**)root->children->data)[0];
    for (int i = 0; i < function_inputs->children->count; i++) {
        AST_node* input = ((AST_node**)function_inputs->children->data)[i];
        if (input->node_type == CONSTANT) {
        } else {
            fprintf(file, "    movl %s, -%i(%%rbp)\n", registers[i], ((varible*)input->data)->stack_pos);
        }
    }
    
    for (int i = 0; i < root->children->count; i++) {
        AST_node* node = ((AST_node**)root->children->data)[i];
        generate_asm_from_node(file, root, node);
    }

    fprintf(file, "end_%s:\n", function_name);
    fprintf(file, "    movq %%rbp, %%rsp\n");
    fprintf(file, "    pop %%rbp\n");
    fprintf(file, "    ret\n");

}


void create_asm_file(FILE* file, dynamic_array* functions)
{
    fprintf(file, "    .section .text\n");
    for (int i = 0; i < functions->count; i++) {
        create_asm_function(file, ((AST_node**)functions->data)[i]);
    }
    
}



