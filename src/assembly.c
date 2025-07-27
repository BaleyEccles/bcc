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
    {"%rax", 8}, {"%eax",  4}, {"%ax",   2}, {"%al",   1}, // 0
    {"%rbx", 8}, {"%ebx",  4}, {"%bx",   2}, {"%bl",   1}, // 1
    {"%rcx", 8}, {"%ecx",  4}, {"%cx",   2}, {"%cl",   1}, // 2
    {"%rdx", 8}, {"%edx",  4}, {"%dx",   2}, {"%dl",   1}, // 3
    {"%rsi", 8}, {"%esi",  4}, {"%si",   2}, {"%sil",  1}, // 4
    {"%rdi", 8}, {"%edi",  4}, {"%di",   2}, {"%dil",  1}, // 5
    {"%rbp", 8}, {"%ebp",  4}, {"%bp",   2}, {"%bpl",  1}, // 6
    {"%rsp", 8}, {"%esp",  4}, {"%sp",   2}, {"%spl",  1}, // 7
    {"%r8",  8}, {"%r8d",  4}, {"%r8w",  2}, {"%r8b",  1}, // 8
    {"%r9",  8}, {"%r9d",  4}, {"%r9w",  2}, {"%r9b",  1}, // 9
    {"%r10", 8}, {"%r10d", 4}, {"%r10w", 2}, {"%r10b", 1}, // 10
    {"%r11", 8}, {"%r11d", 4}, {"%r11w", 2}, {"%r11b", 1}, // 11
    {"%r12", 8}, {"%r12d", 4}, {"%r12w", 2}, {"%r12b", 1}, // 12
    {"%r13", 8}, {"%r13d", 4}, {"%r13w", 2}, {"%r13b", 1}, // 13
    {"%r14", 8}, {"%r14d", 4}, {"%r14w", 2}, {"%r14b", 1}, // 14
    {"%r15", 8}, {"%r15d", 4}, {"%r15w", 2}, {"%r15b", 1}, // 15
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
    fprintf(stderr, "%s:%d: error: Unable to find size for '%c'\n", __FILE__, __LINE__, name);
    return -1;
}

char get_name_from_size(int size)
{
    for (int i = 0; i < SIZE_COUNT; i++) {
        if (size == size_mapping[i].size) {
            return size_mapping[i].name[0];
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find name for %i\n", __FILE__, __LINE__, size);
    *(int*)0 = 0;
    return ' ';
}

char get_size_from_node(AST_node* node)
{
    if (node->node_type == VARIBLE) {
        varible* v = (varible*)node->data;
        for (int i = 0; i < SIZE_COUNT; i++) {
            if (v->type->size == size_mapping[i].size) {
                return size_mapping[i].name[0];
            }
        }
    } else if (node->node_type == CONSTANT) {
        constant* c = (constant*)node->data;
        for (int i = 0; i < SIZE_COUNT; i++) {
            if (c->type->size == size_mapping[i].size) {
                return size_mapping[i].name[0];
            }
        }
        fprintf(stderr, "%s:%d: todo: Unable to get size of constant, it was size %i\n", __FILE__, __LINE__, c->type->size);
    } else if (node->node_type == FUNCTION_CALL) {
        function_call* f = (function_call*)node->data;
        for (int i = 0; i < SIZE_COUNT; i++) {
            if (f->return_type->size == size_mapping[i].size) {
                return size_mapping[i].name[0];
            }
        }
    } else if (node->node_type == OPERATOR) {
        return get_size_from_node(((AST_node**)node->children->data)[0]);
    } else if (node->node_type == ACCESS) {
        return get_size_from_node(((AST_node**)node->children->data)[1]);
    }
    fprintf(stderr, "%s:%d: error: Unable to get size of node, %s %i\n", __FILE__, __LINE__, node->token->data, node->token->pos_in_file);
    *(int*)0 = 0;
    return ' ';
}

char* get_register(const char* input_register, int size)
{
    for (int i = 0; i < REGISTER_COUNT; i++) {
        if (strcmp(input_register, register_mapping[i].name) == 0) {
            for (int j = 4*(i/4); j < 4*(i/4) + REGISTER_WIDTH; j++) {
                if (size == register_mapping[j].size) {
                    return register_mapping[j].name;
                }
            }
        }
    }
    fprintf(stderr, "%s:%d: error: unable to find register for %s with size %i\n", __FILE__, __LINE__, input_register, size);
    *(int*)0 = 0;
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
    *(int*)0 = 0;
    return "NULL";
}

char* resize_register(FILE* file, char* input_register, int from_size, int to_size)
{
    char* output_register = get_register(input_register, to_size);
    if ((from_size == 1 && to_size == 1) ||
        (from_size == 2 && to_size == 2) ||
        (from_size == 4 && to_size == 4) ||
        (from_size == 8 && to_size == 8)) {
        return output_register;
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
        *(int*)0 = 0;
    }
    return output_register;
}


// The following functions work by:
// Evaluating node1
// saving the value from node1
// Evaluating node2
// Applying operation on saved value and node2 value
char asm_plus(FILE* file, AST_node* scope, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, node1);
    int size_1 = get_size_from_name(size_char_1);
    char* reg_1 = get_register("%rax", size_1);

    fprintf(file, "    push %%rax\n");
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    int size_2 = get_size_from_name(size_char_2);
    char* tmp = get_register("%rax", size_2);
    char* reg_2 = get_register("%rbx", size_2);
    fprintf(file, "    mov%c %s, %s\n", size_char_2, tmp, reg_2);
    fprintf(file, "    pop %%rax\n");

    if (size_1 > size_2) {
        reg_2 = resize_register(file, reg_2, size_2, size_1);
        fprintf(file, "    add%c %s, %s\n", size_char_1, reg_2, reg_1);
        return size_char_1;
    } else {
        reg_1 = resize_register(file, reg_1, size_1, size_2);
        fprintf(file, "    add%c %s, %s\n", size_char_2, reg_2, reg_1);
        return size_char_2;
    }
    return ' ';
}

char asm_plus_equal(FILE* file, AST_node* scope, AST_node* node1, AST_node* node2)
{
    char size_char_1 = get_size_from_node(node1);
    char size_char_2 = generate_asm_from_node(file, scope, node2);
    
    int size_1 = get_size_from_name(size_char_1);
    int size_2 = get_size_from_name(size_char_2);
    
    char* dest = get_dest_register_from_size(size_1);
    dest = resize_register(file, dest, size_1, size_2);
    
    generate_asm_from_node(file, scope, node2);
    fprintf(file, "    add%c %s, -%i(%%rbp)\n", size_char_2, dest, ((varible*)node1->data)->stack_pos);

    return size_char_2;
}

char asm_minus(FILE* file, AST_node* scope, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, node1);
    int size_1 = get_size_from_name(size_char_1);
    char* reg_1 = get_register("%rax", size_1);

    fprintf(file, "    push %%rax\n");
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    int size_2 = get_size_from_name(size_char_2);
    char* tmp = get_register("%rax", size_2);
    char* reg_2 = get_register("%rbx", size_2);
    fprintf(file, "    mov%c %s, %s\n", size_char_2, tmp, reg_2);
    fprintf(file, "    pop %%rax\n");

    if (size_1 > size_2) {
        reg_2 = resize_register(file, reg_2, size_2, size_1);
        fprintf(file, "    sub%c %s, %s\n", size_char_1, reg_2, reg_1);
        return size_char_1;
    } else {
        reg_1 = resize_register(file, reg_1, size_1, size_2);
        fprintf(file, "    sub%c %s, %s\n", size_char_2, reg_2, reg_1);
        return size_char_2;
    }
    return ' ';
}
char asm_post_increment(FILE* file, AST_node* scope, AST_node* node1)
{
    char size_char = generate_rvalue_asm(file, scope, node1);
    
    int size = get_size_from_name(size_char);
    
    char* dest = get_dest_register_from_size(size);
    
    if (node1->node_type == CONSTANT) {
        fprintf(file, "    mov%c $%s, %s\n", size_char, ((constant*)node1->data)->value, dest);
        fprintf(file, "    add%c $1, %s\n", size_char, dest);
    } else {
        fprintf(file, "    mov%c -%i(%%rbp), %s\n", size_char, ((varible*)node1->data)->stack_pos, dest);
        fprintf(file, "    add%c $1, %s\n", size_char, dest);
        fprintf(file, "    mov%c %s, -%i(%%rbp)\n", size_char, dest, ((varible*)node1->data)->stack_pos);
    }
    return size_char;
}

char asm_logical_equals(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, node1);
    
    int size_1 = get_size_from_name(size_char_1);
    char* tmp = get_register("%esi", size_1);
    char* dest = get_register("%eax", size_1);
    fprintf(file, "    mov%c %s, %s\n", size_char_1, dest, tmp);
    
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    
    int size_2 = get_size_from_name(size_char_2);
    dest = resize_register(file, dest, size_1, size_2);
    tmp = get_register("%esi", size_2);
    
    fprintf(file, "    cmp%c %s, %s\n", size_char_2, dest, tmp);
    fprintf(file, "    je .logical_equals_true_%i\n", node->token->pos_in_file);
    
    fprintf(file, ".logical_equals_false_%i:\n", node->token->pos_in_file);
    fprintf(file, "    mov%c $0, %s\n", size_char_1, dest);
    fprintf(file, "    jmp .logical_equals_end_%i\n", node->token->pos_in_file);
    fprintf(file, ".logical_equals_true_%i:\n", node->token->pos_in_file);
    fprintf(file, "    mov%c $1, %s\n", size_char_1, dest);
    fprintf(file, ".logical_equals_end_%i:\n", node->token->pos_in_file);
    return size_char_2;
}

char asm_greater_than(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, node1);
    
    int size_1 = get_size_from_name(size_char_1);
    char* tmp = get_register("%esi", size_1);
    char* dest = get_register("%eax", size_1);
    fprintf(file, "    mov%c %s, %s\n", size_char_1, dest, tmp);
    
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    
    int size_2 = get_size_from_name(size_char_2);
    dest = resize_register(file, dest, size_1, size_2);
    tmp = get_register("%esi", size_2);
    
    fprintf(file, "    cmp%c %s, %s\n", size_char_2, dest, tmp);
    fprintf(file, "    jg .greater_than_true_%i\n", node->token->pos_in_file);
    
    fprintf(file, ".greater_than_false_%i:\n", node->token->pos_in_file);
    fprintf(file, "    mov%c $0, %s\n", size_char_1, dest);
    fprintf(file, "    jmp .greater_than_end_%i\n", node->token->pos_in_file);
    fprintf(file, ".greater_than_true_%i:\n", node->token->pos_in_file);
    fprintf(file, "    mov%c $1, %s\n", size_char_1, dest);
    fprintf(file, ".greater_than_end_%i:\n", node->token->pos_in_file);
    return size_char_2;
}

char asm_greater_than_or_equals(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, node1);
    
    int size_1 = get_size_from_name(size_char_1);
    char* tmp = get_register("%esi", size_1);
    char* dest = get_register("%eax", size_1);
    fprintf(file, "    mov%c %s, %s\n", size_char_1, dest, tmp);
    
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    
    int size_2 = get_size_from_name(size_char_2);
    dest = resize_register(file, dest, size_1, size_2);
    tmp = get_register("%esi", size_2);
    
    fprintf(file, "    cmp%c %s, %s\n", size_char_2, dest, tmp);
    fprintf(file, "    jge .greater_than_or_equal_true_%i\n", node->token->pos_in_file);
    
    fprintf(file, ".greater_than_or_equal_false_%i:\n", node->token->pos_in_file);
    fprintf(file, "    mov%c $0, %s\n", size_char_1, dest);
    fprintf(file, "    jmp .greater_than_or_equal_end_%i\n", node->token->pos_in_file);
    fprintf(file, ".greater_than_or_equal_true_%i:\n", node->token->pos_in_file);
    fprintf(file, "    mov%c $1, %s\n", size_char_1, dest);
    fprintf(file, ".greater_than_or_equal_end_%i:\n", node->token->pos_in_file);
    return size_char_2;
}

char asm_less_than(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, node1);
    int size_1 = get_size_from_name(size_char_1);
    char* tmp = get_register("%esi", size_1);
    char* dest = get_register("%eax", size_1);
    fprintf(file, "    mov%c %s, %s\n", size_char_1, dest, tmp);
    
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    
    int size_2 = get_size_from_name(size_char_2);
    dest = resize_register(file, dest, size_1, size_2);
    tmp = get_register("%esi", size_2);
    
    fprintf(file, "    cmp%c %s, %s\n", size_char_2, dest, tmp);
    fprintf(file, "    jl .less_than_true_%i\n", node->token->pos_in_file);
    
    fprintf(file, ".less_than_false_%i:\n", node->token->pos_in_file);
    fprintf(file, "    mov%c $0, %s\n", size_char_1, dest);
    fprintf(file, "    jmp .less_than_end_%i\n", node->token->pos_in_file);
    fprintf(file, ".less_than_true_%i:\n", node->token->pos_in_file);
    fprintf(file, "    mov%c $1, %s\n", size_char_1, dest);
    fprintf(file, ".less_than_end_%i:\n", node->token->pos_in_file);
    return size_char_2;
}

char asm_less_than_or_equals(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, node1);
    
    int size_1 = get_size_from_name(size_char_1);
    char* tmp = get_register("%esi", size_1);
    char* dest = get_register("%eax", size_1);
    fprintf(file, "    mov%c %s, %s\n", size_char_1, dest, tmp);
    
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    
    int size_2 = get_size_from_name(size_char_2);
    dest = resize_register(file, dest, size_1, size_2);
    tmp = get_register("%esi", size_2);
    
    fprintf(file, "    cmp%c %s, %s\n", size_char_2, dest, tmp);
    fprintf(file, "    jle .less_than_or_equal_true_%i\n", node->token->pos_in_file);
    
    fprintf(file, ".less_than_or_equal_false_%i:\n", node->token->pos_in_file);
    fprintf(file, "    mov%c $0, %s\n", size_char_1, dest);
    fprintf(file, "    jmp .less_than_or_equal_end_%i\n", node->token->pos_in_file);
    fprintf(file, ".less_than_or_equal_true_%i:\n", node->token->pos_in_file);
    fprintf(file, "    mov%c $1, %s\n", size_char_1, dest);
    fprintf(file, ".less_than_or_equal_end_%i:\n", node->token->pos_in_file);
    return size_char_2;
}

void asm_branch(FILE* file, char size_char, char* if_true, int if_true_pos, char* if_false, int if_false_pos)
{
    char* reg = get_register("%eax", get_size_from_name(size_char));
    fprintf(file, "    cmp%c $1, %s\n", size_char, reg);
    fprintf(file, "    je %s_%i\n", if_true, if_true_pos);
    fprintf(file, "    jmp %s_%i\n", if_false, if_false_pos);
}

char asm_logical_and(FILE* file, AST_node* scope, AST_node* node, AST_node* node1, AST_node* node2)
{
    char size_char_1 = generate_rvalue_asm(file, scope, node1);
    asm_branch(file, size_char_1, ".AND", node->token->pos_in_file, ".AND_false", node->token->pos_in_file);

    fprintf(file, ".AND_%i:\n", node->token->pos_in_file);
    char size_char_2 = generate_rvalue_asm(file, scope, node2);
    
    asm_branch(file, size_char_2, ".AND_true", node->token->pos_in_file, ".AND_false", node->token->pos_in_file);

    fprintf(file, ".AND_false_%i:\n", node->token->pos_in_file);
    fprintf(file, "    movl $0, %%eax\n");
    fprintf(file, "    jmp .AND_end_%i\n", node->token->pos_in_file);
    fprintf(file, ".AND_true_%i:\n", node->token->pos_in_file);
    fprintf(file, "    movl $1, %%eax\n");
    fprintf(file, "    jmp .AND_end_%i\n", node->token->pos_in_file);
            
    fprintf(file, ".AND_end_%i:\n", node->token->pos_in_file);
    
    return 'l';
}

char asm_varible(FILE* file, AST_node* scope, AST_node* node)
{
    // This code is bad. For some reason removing it stops the struct test from working
    if (((varible*)node->data)->type->type_type == 0) {
        char size_char = get_name_from_size(((varible*)node->data)->type->size);
        int size = get_size_from_name(size_char);
        char* reg = get_register("%eax", size);
        fprintf(file, "    mov%c -%i(%%rbp), %s \n", size_char, ((varible*)node->data)->stack_pos, reg);
        return size_char;
    }
    if (((varible*)node->data)->type->type_type == TYPE) {
        // TODO: this is a patch job, I dont understand what I have done
        
        char size_char = get_name_from_size(((varible*)node->data)->type->size);
        int size = get_size_from_name(size_char);
        char* reg = get_register("%eax", size);
        fprintf(file, "    mov%c -%i(%%rbp), %s \n", size_char, ((varible*)node->data)->stack_pos, reg);
        return size_char;
    }
    return '!';
}

char generate_rvalue_asm(FILE* file, AST_node* scope, AST_node* node)
{
    char output;
    switch (node->node_type) {
    case OPERATOR: {
        AST_node* child1 = ((AST_node**)node->children->data)[0];
        AST_node* child2 = ((AST_node**)node->children->data)[1];
        
        TOKEN_TYPE t = ((operator*)node->data)->type;
        switch (t) {
        case PLUS: {
            fprintf(file, "    #HERE\n");
            output = asm_plus(file, scope, child1, child2);
            fprintf(file, "    #HERE2\n");
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
            output = asm_post_increment(file, scope, child1);
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
        break;
    }

    case FUNCTION_CALL: {
        output = generate_asm_function_call(file, scope, node);
        break;
    }
    case VARIBLE: {
        output = asm_varible(file, scope, node);
        break;
    }
    case CONSTANT: {
        if (type_is_string(((constant*)node->data)->type)) {
            fprintf(file, "    leaq .STR_%s_%i(%%rip), %%rax\n", ((function*)scope->data)->name, ((function*)scope->data)->str_count);
            output = ' ';
        } else {
            output = get_name_from_size(((constant*)node->data)->type->size);
            int size = get_size_from_name(output);
            char* reg = get_register("%eax", size);
            fprintf(file, "    mov%c $%s, %s\n", output, ((constant*)node->data)->value, reg);
        }
        break;
    }
    case CAST: {
        output = generate_asm_from_node(file, scope, node);
        break;
    }
    case ACCESS: {
        output = generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
        break;
    }
    default: {
        fprintf(stderr, "%s:%d: TODO: Generating rvalue for %i is not done yet\n", __FILE__, __LINE__, node->node_type);
        break;
    }
    }
    return output;
}


void generate_if_asm(FILE* file, AST_node* scope, AST_node* node)
{
    if (node->children->count == 3) {
        AST_node* else_node = ((AST_node**)node->children->data)[2];
        
        char size_char_1 = generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
        asm_branch(file, size_char_1, ".if", node->token->pos_in_file, ".else", else_node->token->pos_in_file);
        
        fprintf(file, ".if_%i:\n", node->token->pos_in_file);
        generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
        fprintf(file, "    jmp .end_if_%i\n", node->token->pos_in_file);
        fprintf(file, ".else_%i:\n", else_node->token->pos_in_file);
        generate_asm_from_node(file, scope, else_node);
    } else if (node->children->count == 2) {

        char size_char_1 = generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
        asm_branch(file, size_char_1, ".if", node->token->pos_in_file, ".end_if", node->token->pos_in_file);

        fprintf(file, ".if_%i:\n", node->token->pos_in_file);
        generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
        fprintf(file, "    jmp .end_if_%i\n", node->token->pos_in_file);
    }
    fprintf(file, ".end_if_%i:\n", node->token->pos_in_file);
    
}

void generate_for_asm(FILE* file, AST_node* scope, AST_node* node)
{
    generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[0]);
    fprintf(file, ".start_for_%i:\n", node->token->pos_in_file);
    generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[3]);
    generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[2]);
    char size_char = generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[1]);
    asm_branch(file, size_char, ".start_for", node->token->pos_in_file, ".end_for", node->token->pos_in_file);
    fprintf(file, ".end_for_%i:\n", node->token->pos_in_file);
}

void generate_while_asm(FILE* file, AST_node* scope, AST_node* node)
{
    fprintf(file, ".start_while_%i:\n", node->token->pos_in_file);
    generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
    char size_char = generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
    asm_branch(file, size_char, ".start_while", node->token->pos_in_file, ".end_while", node->token->pos_in_file);
    fprintf(file, ".end_while_%i:\n", node->token->pos_in_file);
}


// https://en.wikipedia.org/wiki/X86_calling_conventions#x86-64_calling_conventions
#define CALLING_REGISTERS_COUNT (6)
static const char* registers[] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

char generate_asm_function_call(FILE* file, AST_node* scope, AST_node* node)
{
    for (int i = 0; i < node->children->count; i++) {
        AST_node* child = ((AST_node**)node->children->data)[i];
        char size_char = generate_rvalue_asm(file, scope, child);
        char size = get_size_from_name(size_char);
        char* input_reg = get_register("%eax", size);
        if (i < CALLING_REGISTERS_COUNT) {
            char* reg = get_register(registers[i], size);
            fprintf(file, "    mov%c %s, %s\n", size_char, input_reg, reg);
            
        } else {
            fprintf(stderr, "%s:%d: TODO: Calling function with more than 6 inputs is not done yet\n \n", __FILE__, __LINE__);
        }
    }
    fprintf(file, "    call %s\n", ((function_call*)node->data)->name);
    
    char output_char = get_name_from_size((((function_call*)node->data)->return_type->size));
    return output_char;
}

char generate_asm_cast(FILE* file, AST_node* scope, AST_node* node)
{
    char char_size = generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[0]);
    
    int from_size = get_size_from_name(char_size);
    int to_size = ((cast*)node->data)->to_type->size;

    char* dest = get_register("%eax", from_size);
    resize_register(file, dest, from_size, to_size);
    return get_name_from_size(to_size);
}


char generate_asm_access(FILE* file, AST_node* scope, AST_node* node)
{
    char size_char_1 = generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[1]);
    int size_1 = get_size_from_name(size_char_1);
    char* dest = get_register("%eax", size_1);

    char* tmp = get_register("%ebx", size_1);

    fprintf(file, "    mov%c %s, %s\n", size_char_1, dest, tmp);
    
    dest = resize_register(file, tmp, size_1, 8);
    
    char size_char_2 = generate_asm_from_node(file, scope, ((AST_node**)node->children->data)[0]);
    int size_2 = get_size_from_name(size_char_2);
    dest = get_register("%eax", size_2);
    dest = resize_register(file, dest, size_2, 8);
    
    fprintf(file, "    addq %%rbx, %%rax\n");

    AST_node* n = ((AST_node**)node->children->data)[0];
    type* t = get_dereferenced_type(scope->types, ((varible*)n->data)->type);
    dest = get_register("%eax", t->size);
    fprintf(file, "    mov (%%rax), %s\n", dest);

    char output = get_name_from_size(t->size);
    return output;
}

void generate_access_asm(FILE* file, AST_node* scope, AST_node* node, char size_char_1)
{
    
    if (node->node_type == VARIBLE) {
        if (size_char_1 == ' ') {
            size_char_1 = get_size_from_node(node);
        }
        int size = get_size_from_name(size_char_1);
        char* reg = get_register("%eax", size);    
        fprintf(file, "    mov%c %s, -%i(%%rbp)\n", size_char_1, reg, ((varible*)(node->data))->stack_pos);
    } else if (node->node_type == ACCESS) {
        generate_access_asm(file, scope, ((AST_node**)node->children->data)[1], size_char_1);
    } else {
        fprintf(stderr, "%s:%d: todo: Accessing %s %i with type %i is not handled yey\n", __FILE__, __LINE__, node->token->data, node->token->pos_in_file, node->node_type);
    }
}

char generate_asm_from_node(FILE* file, AST_node* scope, AST_node* node)
{
    char output;
    if (node->node_type == OPERATOR && ((operator*)node->data)->type == EQUALS) {
        output = generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[1]);
        generate_access_asm(file, scope,  ((AST_node**)node->children->data)[0], output);
        

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
                    
                    char output = generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
                    int size = get_size_from_name(output);
                    char* dest = get_register("%eax", size);

                    dest = resize_register(file, dest, size, 8);
                    
                    fprintf(file, "    movq %s, %%rdi\n", dest);
                    fprintf(file, "    movq $60, %%rax\n");
                    fprintf(file, "    syscall\n");
                }
                else {
                    // Do function return
                    output = generate_rvalue_asm(file, scope, ((AST_node**)node->children->data)[0]);
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
    return output;
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
            int size = ((varible*)input->data)->type->size;
            char size_char = get_name_from_size(size);
            char* reg = get_register(registers[i], size);
            fprintf(file, "    mov%c %s, -%i(%%rbp)\n", size_char, reg, ((varible*)input->data)->stack_pos);
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



