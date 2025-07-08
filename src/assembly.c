#include "assembly.h"

typedef struct {
    char* name;
    int size;
} name_size_mapping;

// https://web.stanford.edu/class/cs107/resources/x86-64-reference.pdf
#define REGISTER_WIDTH (4)
#define REGISTER_HEIGHT (16)
#define REGISTER_COUNT (REGISTER_HEIGHT*REGISTER_WIDTH)
static const name_size_mapping register_mapping[] = {
    {"%%rax", 8}, {"%%eax",  4}, {"%%ax",   2}, {"%%al",   1}, // 0
    {"%%rbx", 8}, {"%%ebx",  4}, {"%%bx",   2}, {"%%bl",   1}, // 1
    {"%%rcx", 8}, {"%%ecx",  4}, {"%%cx",   2}, {"%%cl",   1}, // 2
    {"%%rdx", 8}, {"%%edx",  4}, {"%%dx",   2}, {"%%dl",   1}, // 3
    {"%%rsi", 8}, {"%%esi",  4}, {"%%si",   2}, {"%%sil",  1}, // 4
    {"%%rdi", 8}, {"%%edi",  4}, {"%%di",   2}, {"%%dil",  1}, // 5
    {"%%rbp", 8}, {"%%ebp",  4}, {"%%bp",   2}, {"%%bpl",  1}, // 6
    {"%%rsp", 8}, {"%%esp",  4}, {"%%sp",   2}, {"%%spl",  1}, // 7
    {"%%r8",  8}, {"%%r8d",  4}, {"%%r8w",  2}, {"%%r8b",  1}, // 8
    {"%%r9",  8}, {"%%r9d",  4}, {"%%r9w",  2}, {"%%r9b",  1}, // 9
    {"%%r10", 8}, {"%%r10d", 4}, {"%%r10w", 2}, {"%%r10b", 1}, // 10
    {"%%r11", 8}, {"%%r11d", 4}, {"%%r11w", 2}, {"%%r11b", 1}, // 11
    {"%%r12", 8}, {"%%r12d", 4}, {"%%r12w", 2}, {"%%r12b", 1}, // 12
    {"%%r13", 8}, {"%%r13d", 4}, {"%%r13w", 2}, {"%%r13b", 1}, // 13
    {"%%r14", 8}, {"%%r14d", 4}, {"%%r14w", 2}, {"%%r14b", 1}, // 14
    {"%%r15", 8}, {"%%r15d", 4}, {"%%r15w", 2}, {"%%r15b", 1}, // 15
};

#define SIZE_COUNT 4
static const name_size_mapping size_mapping[] = {
    {"q", 8}, {"l", 4}, {"w", 2}, {"b", 1},
};

int get_size_from_name(char name)
{
    for (int i = 0; i < SIZE_COUNT; i++) {
        if (name == size_mapping[i].name[0]) {
            return size_mapping[i].size;
        }
    }
}

char get_name_from_size(int size)
{
    for (int i = 0; i < SIZE_COUNT; i++) {
        if (size == size_mapping[i].size) {
            return size_mapping[i].name[0];
        }
    }
}

char get_size_from_node(AST_node* node)
{
    if (node->node_type == VARIBLE) {
        varible* v = (varible*)node->data
        for (int i = 0; i < SIZE_COUNT; i++) {
            if (v->type->size == size_mapping[i].size) {
                return size_mapping[i].name[0];
            }
        }
        fprintf(stderr, "%s:%d: todo: Unable to get size of varible, it was size %i \n    Probably because it is a struct and they have not been handled yet\n", __FILE__, __LINE__, v->type->size);
    } else if (node->node_type == CONSTANT) {
        constant* c = (constant*)node->data;
        for (int i = 0; i < SIZE_COUNT; i++) {
            if (c->type->size == size_mapping[i].size) {
                return size_mapping[i].name[0];
            }
        }
        fprintf(stderr, "%s:%d: todo: Unable to get size of constant, it was size %i\n", __FILE__, __LINE__, v->type->size);
    }
}

char* get_register(char* input_register, int size)
{
    for (int i = 0; i < REGISTER_COUNT; i++) {
        if (strcmp(input_register, register_mapping[i].name) == 0) {
            for (int j = 4*(i/4); j < 4*(i/4) + REGISTER_WIDTH; j++) {
                if (size == register_mapping[j].size) {
                    return register_mapping[j].name
                }
            }
        }
    }
    fprintf(stderr, "%s:%d: error: unable to find register for %s\n", __FILE__, __LINE__, input_register);
    return "NULL";
}

char* get_dest_register_from_size(int size)
{
    for (int i = 0; i < REGISTER_WIDTH; i++) {
        if (size == register_mapping[i].size) {
            return register_mapping[i].name;
        }
    }
    fprintf(stderr, "%s:%d: error: unable to find rax register for size %i\n", __FILE__, __LINE__, size);
}

char* resize_register(char* input_register, int from_size, int to_size)
{
    char* output_register = get_register(input_register, to_size)
    if ((from_size == 1 && to_size == 1) ||
        (from_size == 2 && to_size == 2) ||
        (from_size == 4 && to_size == 4) ||
        (from_size == 8 && to_size == 8)) {
        return;
    } else if (from_size == 1 && to_size == 2) {
        fprintf(file, "    movsb %s, %s\n", input_register, output_register);
    } else if (from_size == 1 && to_size == 4) {
        fprintf(file, "    movsbl %s, %s\n", input_register, output_register);
    } else if (from_size == 1 && to_size == 8) {
        fprintf(file, "    movsbq %s, %s\n", input_register, output_register);
    } else if (from_size == 2 && to_size == 1) {
        fprintf(file, "    movsw %s, %s\n", input_register, output_register);
        fprintf(stderr, "%s:%d: warning: Resizeing from %i to %i, this may loose data\n", __FILE__, __LINE__, from_size, to_size);
    } else if (from_size == 2 && to_size == 4) {
        fprintf(file, "    movsw %s, %s\n", input_register, output_register);
    } else if (from_size == 2 && to_size == 8) {
        fprintf(file, "    movsw %s, %s\n", input_register, output_register);
    } else if (from_size == 4 && to_size == 1) {
        fprintf(file, "    movb %s, %s\n", input_register, output_register);
        fprintf(stderr, "%s:%d: warning: Resizeing from %i to %i, this may loose data\n", __FILE__, __LINE__, from_size, to_size);
    } else if (from_size == 4 && to_size == 2) {
        fprintf(file, "    movw %s, %s\n", input_register, output_register);
        fprintf(stderr, "%s:%d: warning: Resizeing from %i to %i, this may loose data\n", __FILE__, __LINE__, from_size, to_size);
    } else if (from_size == 4 && to_size == 8) {
        fprintf(file, "    movsl %s, %s\n", input_register, output_register);
    } else if (from_size == 8 && to_size == 1) {
        fprintf(file, "    movb %s, %s\n", input_register, output_register);
        fprintf(stderr, "%s:%d: warning: Resizeing from %i to %i, this may loose data\n", __FILE__, __LINE__, from_size, to_size);
    } else if (from_size == 8 && to_size == 2) {
        fprintf(file, "    movw %s, %s\n", input_register, output_register);
        fprintf(stderr, "%s:%d: warning: Resizeing from %i to %i, this may loose data\n", __FILE__, __LINE__, from_size, to_size);
    } else if (from_size == 8 && to_size == 4) {
        fprintf(stderr, "%s:%d: warning: Resizeing from %i to %i, this may loose data\n", __FILE__, __LINE__, from_size, to_size);
        fprintf(file, "    movl %s, %s\n", input_register, output_register);
    } else {
        fprintf(stderr, "%s:%d: error: Unable to resize from %i to %i\n", __FILE__, __LINE__, from_size, to_size);
    }
    return output_register;
}

char asm_plus(FILE* file, AST_node* scope, AST_node* node1, AST_node* node2)
{
    char size_char_1 = get_size_from_node(node1);
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    
    int size_1 = get_size_from_name(size_char_1);
    int size_2 = get_size_from_name(size_char_2);
    
    char* dest = get_dest_register_from_size(size_1);
    dest = resize_register(dest, size_1, size_2);
    
    if (node1->node_type == CONSTANT) {
        fprintf(file, "    add%c $%s, %s\n", size_char_2, ((constant*)node1->data)->value, dest);
    } else if (node1->node_type == FUNCTION_CALL) {
        char* tmp = get_register("%%edi", size_2);
        fprintf(file, "    push %%rax\n");
        generate_asm_from_node(file, scope, node1);
        fprintf(file, "    mov%c %s, %s\n", size_char_2, dest, tmp);
        fprintf(file, "    pop %%rax\n");
        fprintf(file, "    add%c %s, %s\n", size_char_2, tmp, dest);
    } else {
        fprintf(file, "    add%c -%i(%%rbp), %s\n", size_char_2 ((varible*)node1->data)->stack_pos, dest);
    }
    return size_char_2;
}

char asm_plus_equal(FILE* file, AST_node* scope, AST_node* node1, AST_node* node2)
{
    char size_char_1 = get_size_from_node(node1);
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    
    int size_1 = get_size_from_name(size_char_1);
    int size_2 = get_size_from_name(size_char_2);
    
    char* dest = get_dest_register_from_size(size_1);
    dest = resize_register(dest, size_1, size_2);
    
    generate_asm_from_node(file, scope, child2);
    
    fprintf(file, "    add%c %s, -%i(%%rbp)\n", size_2, dest, ((varible*)child1->data)->stack_pos);
    return size_char_2;
}

char asm_minus(FILE* file, AST_node* scope, AST_node* node1, AST_node* node2)
{
    char size_char_1 = get_size_from_node(node1);
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    
    int size_1 = get_size_from_name(size_char_1);
    int size_2 = get_size_from_name(size_char_2);
    
    char* dest = get_dest_register_from_size(size_1);
    dest = resize_register(dest, size_1, size_2);
    
    if (node1->node_type == CONSTANT) {
        fprintf(file, "    sub%c $%s, %s\n", size_char_2, ((constant*)node1->data)->value, dest);
    } else if (node1->node_type == FUNCTION_CALL) {
        char* tmp = get_register("%%edi", size_2);
        fprintf(file, "    push %%rax\n");
        generate_asm_from_node(file, scope, node1);
        fprintf(file, "    mov%c %s, %s\n", size_char_2, dest, tmp);
        fprintf(file, "    pop %%rax\n");
        fprintf(file, "    sub%c %s, %s\n", size_char_2, tmp, dest);
    } else {
        fprintf(file, "    sub%c -%i(%%rbp), %s\n", size_char_2 ((varible*)node1->data)->stack_pos, dest);
    }
    return size_char_2;
}
char asm_post_increment(FILE* file, AST_node* scope, AST_node* node1, AST_node* node2)
{
    char size_char_1 = get_size_from_node(node1);
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    
    int size_1 = get_size_from_name(size_char_1);
    int size_2 = get_size_from_name(size_char_2);
    
    char* dest = get_dest_register_from_size(size_1);
    dest = resize_register(dest, size_1, size_2);
    
    if (child1->node_type == CONSTANT) {
        fprintf(file, "    mov%c $%s, %s\n", size_char_2, ((constant*)child1->data)->value, dest);
        fprintf(file, "    add%c $1, %s\n", size_char_2, dest);
    } else {
        fprintf(file, "    mov%c -%i(%%rbp), %s\n", size_char_2, ((varible*)child1->data)->stack_pos, dest);
        fprintf(file, "    add%c $1, %s\n", size_char_2, dest);
        fprintf(file, "    mov%c %s, -%i(%%rbp)\n", size_char_2, dest, ((varible*)child1->data)->stack_pos);
    }
    return size_char_2;
}

char asm_logical_equals(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, child1);
    
    int size_1 = get_size_from_name(size_char_1);
    char* tmp = get_register("%%esi", size_1);
    char* dest = get_register("%%eax", size_1);
    fprintf(file, "    mov%c %s, %s\n", size_char_1, dest, tmp);
    
    char size_char_2 = generate_rvalue_asm(file, scope, child2);
    
    int size_2 = get_size_from_name(size_char_2);
    dest = resize_register(dest, size_1, size_2);
    tmp = get_register("%%esi", size_2);
    
    fprintf(file, "    cmp%c %s, %s\n", size_char_2, dest, tmp);
    fprintf(file, "    je .logical_equals_true_%i", node->token->pos_in_file);
    
    fprintf(file, ".logical_equals_false_%i:", node->token->pos_in_file);
    fprintf(file, "    mov%c $0, %s\n", size_char_1, dest);
    fprintf(file, "    jmp .logical_equals_end_%i", node->token->pos_in_file);
    fprintf(file, ".logical_equals_true_%i:", node->token->pos_in_file);
    fprintf(file, "    mov%c $1, %s\n", size_char_1, dest);
    fprintf(file, ".logical_equals_end_%i:", node->token->pos_in_file);
    return size_char_2;
}

char asm_greater_than(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, child1);
    
    int size_1 = get_size_from_name(size_char_1);
    char* tmp = get_register("%%esi", size_1);
    char* dest = get_register("%%eax", size_1);
    fprintf(file, "    mov%c %s, %s\n", size_char_1, dest, tmp);
    
    char size_char_2 = generate_rvalue_asm(file, scope, child2);
    
    int size_2 = get_size_from_name(size_char_2);
    dest = resize_register(dest, size_1, size_2);
    tmp = get_register("%%esi", size_2);
    
    fprintf(file, "    cmp%c %s, %s\n", size_char_2, dest, tmp);
    fprintf(file, "    jg .greater_than_true_%i", node->token->pos_in_file);
    
    fprintf(file, ".greater_than_false_%i:", node->token->pos_in_file);
    fprintf(file, "    mov%c $0, %s\n", size_char_1, dest);
    fprintf(file, "    jmp .greater_than_end_%i", node->token->pos_in_file);
    fprintf(file, ".greater_than_true_%i:", node->token->pos_in_file);
    fprintf(file, "    mov%c $1, %s\n", size_char_1, dest);
    fprintf(file, ".greater_than_end_%i:", node->token->pos_in_file);
    return size_char_2;
}

char asm_greater_than_or_equals(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, child1);
    
    int size_1 = get_size_from_name(size_char_1);
    char* tmp = get_register("%%esi", size_1);
    char* dest = get_register("%%eax", size_1);
    fprintf(file, "    mov%c %s, %s\n", size_char_1, dest, tmp);
    
    char size_char_2 = generate_rvalue_asm(file, scope, child2);
    
    int size_2 = get_size_from_name(size_char_2);
    dest = resize_register(dest, size_1, size_2);
    tmp = get_register("%%esi", size_2);
    
    fprintf(file, "    cmp%c %s, %s\n", size_char_2, dest, tmp);
    fprintf(file, "    jge .greater_than_or_equal_true_%i", node->token->pos_in_file);
    
    fprintf(file, ".greater_than_or_equal_false_%i:", node->token->pos_in_file);
    fprintf(file, "    mov%c $0, %s\n", size_char_1, dest);
    fprintf(file, "    jmp .greater_than_or_equal_end_%i", node->token->pos_in_file);
    fprintf(file, ".greater_than_or_equal_true_%i:", node->token->pos_in_file);
    fprintf(file, "    mov%c $1, %s\n", size_char_1, dest);
    fprintf(file, ".greater_than_or_equal_end_%i:", node->token->pos_in_file);
    return size_char_2;
}

char asm_less_than(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, child1);
    
    int size_1 = get_size_from_name(size_char_1);
    char* tmp = get_register("%%esi", size_1);
    char* dest = get_register("%%eax", size_1);
    fprintf(file, "    mov%c %s, %s\n", size_char_1, dest, tmp);
    
    char size_char_2 = generate_rvalue_asm(file, scope, child2);
    
    int size_2 = get_size_from_name(size_char_2);
    dest = resize_register(dest, size_1, size_2);
    tmp = get_register("%%esi", size_2);
    
    fprintf(file, "    cmp%c %s, %s\n", size_char_2, dest, tmp);
    fprintf(file, "    jl .less_than_true_%i", node->token->pos_in_file);
    
    fprintf(file, ".less_than_false_%i:", node->token->pos_in_file);
    fprintf(file, "    mov%c $0, %s\n", size_char_1, dest);
    fprintf(file, "    jmp .less_than_end_%i", node->token->pos_in_file);
    fprintf(file, ".less_than_true_%i:", node->token->pos_in_file);
    fprintf(file, "    mov%c $1, %s\n", size_char_1, dest);
    fprintf(file, ".less_than_end_%i:", node->token->pos_in_file);
    return size_char_2;
}

char asm_less_than_or_equals(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, child1);
    
    int size_1 = get_size_from_name(size_char_1);
    char* tmp = get_register("%%esi", size_1);
    char* dest = get_register("%%eax", size_1);
    fprintf(file, "    mov%c %s, %s\n", size_char_1, dest, tmp);
    
    char size_char_2 = generate_rvalue_asm(file, scope, child2);
    
    int size_2 = get_size_from_name(size_char_2);
    dest = resize_register(dest, size_1, size_2);
    tmp = get_register("%%esi", size_2);
    
    fprintf(file, "    cmp%c %s, %s\n", size_char_2, dest, tmp);
    fprintf(file, "    jle .less_than_or_equal_true_%i", node->token->pos_in_file);
    
    fprintf(file, ".less_than_or_equal_false_%i:", node->token->pos_in_file);
    fprintf(file, "    mov%c $0, %s\n", size_char_1, dest);
    fprintf(file, "    jmp .less_than_or_equal_end_%i", node->token->pos_in_file);
    fprintf(file, ".less_than_or_equal_true_%i:", node->token->pos_in_file);
    fprintf(file, "    mov%c $1, %s\n", size_char_1, dest);
    fprintf(file, ".less_than_or_equal_end_%i:", node->token->pos_in_file);
    return size_char_2;
}

void asm_branch(char size, char* if_true, int if_true_pos, char* if_false, int if_false_pos)
{
    char* reg = get_register("%%eax", size);
    fprintf(file, "    cmp%c $1, %s\n", size, reg);
    fprintf(file, "    je %s_%i", if_true, if_true_pos);
    fprintf(file, "    jmp %s_%i", if_false, if_false_pos);
}

char asm_logical_and(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, child1);
    asm_branch(size_char_1, "AND", node->token->pos_in_file, "AND_false", node->token->pos_in_file);

    fprintf(file, "AND_%i:\n", node->token->pos_in_file);
    char size_char_2 = generate_rvalue_asm(file, scope, child2);
    
    asm_branch(size_char_2, "AND_true", node->token->pos_in_file, "AND_false", node->token->pos_in_file);

    fprintf(file, "AND_false_%i:\n", node->token->pos_in_file);
    fprintf(file, "    movl $0, %%eax\n");
    fprintf(file, "    jmp AND_end_%i\n", node->token->pos_in_file);
    fprintf(file, "AND_true_%i:\n", node->token->pos_in_file);
    fprintf(file, "    movl $1, %%eax\n");
    fprintf(file, "    jmp AND_end_%i\n", node->token->pos_in_file);
            
    fprintf(file, "AND_end_%i:\n", node->token->pos_in_file);
    
    return 'l';
}
char asm_varible(FILE* file, AST_node* scope, AST_node* node)
{
    char size_char = get_name_from_size(((varible*)node->data)->type->size);
    int size = get_size_from_name(size_char);
    char* reg = get_register("%%eax", size);
    fprintf(file, "    mov%c -%i(%%rbp), %s\n", size_char, ((varible*)node->data)->stack_pos, reg);
    return size_char;
}

char generate_rvalue_asm(FILE* file, AST_node* scope, AST_node* node)
{
    char output = NULL;
    if (node->node_type == OPERATOR) {
        AST_node* child1 = ((AST_node**)node->children->data)[0];
        AST_node* child2 = ((AST_node**)node->children->data)[1];
        
        TOKEN_TYPE t = ((operator*)node->data)->type;
        switch (t) {
        case PLUS: {
            output = asm_plus(file, scope, child1, child2);
            break;
        }
        case PLUS_EQUALS: {
            output = asm_plus_equal(file, scope, child1, child2);
            break;
        }
        case MINUS: {
            output = asm_minus(file, scope, child1, child2);
            break;
        }
        case POST_INCREMENT: {
            output = asm_post_increment(file, scope, child1, child2);
            break;
        }
        case LOGICAL_EQUALS: {
            output = asm_logical_equals(file, scope, node, child1, child2); 
            break;
        }
        case GREATER_THAN: {
            output = asm_greater_than(file, scope, node, child1, child2);
            break;
        }
        case GREATER_THAN_OR_EQUALS: {
            output = asm_greater_than_or_equals(file, scope, node, child1, child2);
            break;
        }
        case LESS_THAN: {
            output = asm_less_than(file, scope, node, child1, child2);
            break;
        }
        case LESS_THAN_OR_EQUALS: {
            output = asm_less_than_or_equals(file, scope, node, child1, child2);
            break;
        }
        case LOGICAL_AND: {
            output = asm_logical_and(file, scope, node, child1, child2);

            break;
        }
        default: {
        }
            fprintf(stderr, "%s:%d: TODO: Operator %s %i was not handled\n", __FILE__, __LINE__, node->token->data, node->token->pos_in_file);
            fprintf(file  , "%s:%d: TODO: Operator %s %i was not handled\n", __FILE__, __LINE__, node->token->data, node->token->pos_in_file);
        }
        
        
    }
    if (node->node_type == FUNCTION_CALL) {
        output = generate_asm_function_call(file, scope, node);
    }
    if (node->node_type == VARIBLE) {
        output = asm_varible(file, scope, node);
        
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
    "%%rdi", "%%rsi", "%%rdx", "%%rcx", "%%r8", "%%r9"
};

char generate_asm_function_call(FILE* file, AST_node* scope, AST_node* node)
{
    for (int i = 0; i < node->children->count; i++) {
        AST_node* child = ((AST_node**)node->children->data)[i];
        char size_char = generate_rvalue_asm(file, scope, child);
        char size = get_size_from_name(size_char);
        if (i < CALLING_REGISTERS_COUNT) {
            char* reg = get_register(registers[i], size);
            fprintf(file, "    mov%c %s, %s\n", size_char,  reg);
            
        } else {
            fprintf(stderr, "%s:%d: TODO: Calling function with more than 6 inputs is not done yet\n \n", __FILE__, __LINE__);
        }
    }
    fprintf(file, "    call %s\n", ((function_call*)node->data)->name);
    
    char output_char = get_name_from_size((((function_call*)node->data)->return_type->size))[0];
    return output_char;
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
        fprintf(file, "    movsb %%al, %%ax\n");
    } else if (from_size == 1 && to_size == 4) {
        fprintf(file, "    movsbl %%al, %%eax\n");
    } else if (from_size == 1 && to_size == 8) {
        fprintf(file, "    movsbq %%al, %%rax\n");
    } else if (from_size == 2 && to_size == 1) {
        fprintf(file, "    movsw %%ax, %%al\n");
    } else if (from_size == 2 && to_size == 4) {
        fprintf(file, "    movsw %%ax, %%eax\n");
    } else if (from_size == 2 && to_size == 8) {
        fprintf(file, "    movsw %%ax, %%rax\n");
    } else if (from_size == 4 && to_size == 1) {
        fprintf(file, "    movb %%eax, %%al\n");
    } else if (from_size == 4 && to_size == 2) {
        fprintf(file, "    movw %%eax, %%ax\n");
    } else if (from_size == 4 && to_size == 8) {
        fprintf(file, "    movsl %%eax, %%rax\n");
    } else if (from_size == 8 && to_size == 1) {
        fprintf(file, "    movb %%rax, %%al\n");
    } else if (from_size == 8 && to_size == 2) {
        fprintf(file, "    movw %%rax, %%ax\n");
    } else if (from_size == 8 && to_size == 4) {
        fprintf(file, "    movl %%rax, %%eax\n");
    } else {
        fprintf(stderr, "%s:%d: error: Unable to cast from %i to %i, token %s, %i\n", __FILE__, __LINE__, from_size, to_size, node->token->data, node->token->pos_in_file);
        *(int*)0 = 0;
    }
}


char generate_asm_access(FILE* file, AST_node* scope, AST_node* node)
{
    char size_char_1 = generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
    int size_1 = get_size_from_name(size_char_1);
    char* dest = get_register("%%eax", size_1);
    fprintf(file, "    movs%cq %s, %%rbx\n", size_char, dest);
    
    char size_char_2 = generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[0]);
    int size_2 = get_size_from_name(size_char_2);
    dest = get_register("%%eax", size_2);
    fprintf(file, "    movs%cq %s, %%rax\n", size_char, dest);
    
    fprintf(file, "    addq %%rbx, %%rax\n");

    type* t = get_dereferenced_type(scope->types, ((varible*)node->data)->type);
    dest = get_register("%%eax", t->size);
    fprintf(file, "    mov (%%rax), %s\n", dest);
}

void generate_asm_from_node(FILE* file, AST_node* scope, AST_node* node)
{
    char output;
    if (node->node_type == OPERATOR && ((operator*)node->data)->type == EQUALS) {
        output = generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[1]);
        fprintf(file, "    movl %%eax, -%i(%%rbp)\n", ((varible*)((AST_node**)node->children->data)[0]->data)->stack_pos);
    } else if (node->node_type == FUNCTION_CALL) {
        output = generate_asm_function_call(file, scope, node);
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
        output = generate_rvalue_asm(file, scope, node);
    } else if (node->node_type == ACCESS) {
        output = generate_asm_access(file, scope, node);
    } else if (node->node_type == CAST) {
        output = generate_asm_cast(file, scope, node);
    } else if (node->node_type == VARIBLE) {
        output = generate_rvalue_asm(file, scope, node);
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



