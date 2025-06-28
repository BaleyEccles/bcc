#include "AST.h"

void init_AST_node(AST_node* node)
{
    dynamic_array* node_da = malloc(sizeof(dynamic_array));
    da_init(node_da, AST_node);
    node->children = node_da;
}

AST_node* get_node_from_pos_recursive(AST_node* node, int pos) {
    if (node->token->pos_in_file == pos) {
        return node;
    }

    for (int i = 0; i < node->children->count; i++) {
        if (get_node_from_pos_recursive(((AST_node**)node->children->data)[i], pos) != NULL) {
            return get_node_from_pos_recursive(((AST_node**)node->children->data)[i], pos);
        }
    }
    return NULL;
}

AST_node* get_node_from_pos(AST_node* node, int pos) {
    AST_node* output = get_node_from_pos_recursive(node, pos);
    if (output == NULL) {
        fprintf(stderr, "%s:%d: error: Unable to find node from posistion %i\n", __FILE__, __LINE__, pos);
    }
    return output;
}

AST_node* get_node_from_name_recursive(AST_node* node, char* name) {
    if (node->node_type == VARIBLE) {
        if (strcmp(((varible*)node->data)->name, name) == 0) {
            return node;
        }
    }
    for (int i = 0; i < node->children->count; i++) {
        // TODO: scope stuff
        //     Example: only go into nodes if they arent if, for, while etc
        if (get_node_from_name_recursive(((AST_node**)node->children->data)[i], name) != NULL) {
            return get_node_from_name_recursive(((AST_node**)node->children->data)[i], name);
        }
    }
    return NULL;
}


AST_node* get_node_from_name(AST_node* node, char* name) {
    AST_node* output = get_node_from_name_recursive(node, name);
    if (output == NULL) {
        fprintf(stderr, "%s:%d: error: Unable to find varible from name %s\n", __FILE__, __LINE__, name);
    }
    return output;
}


bool is_varible_defined(AST_node* node, char* str) {
    if (node->node_type == VARIBLE) {
        char* varible_name = ((varible*)node->data)->name;
        if (strcmp(varible_name, str) == 0) {
            return true;
        }
    }
    if (node->node_type == FUNCTION_INPUT) {
        for (int i = 0; i < node->children->count; i++) {
            char* input_name = ((varible*)(((AST_node**)node->children->data)[i]->data))->name;
            if (strcmp(input_name, str) == 0) {
                return true;
            }   
        }            
    }
    for (int i = 0; i < node->children->count; i++) {
        // TODO: scope stuff
        //     Example: only go into nodes if they arent if, for, while etc
        if (is_varible_defined(((AST_node**)node->children->data)[i], str)) {
            return true;
        }
    }
    return false;
}


AST_node* create_constant_node(AST_node* scope, dynamic_array* tokens, int start, int end)
{
    if (end - start > 1) {
        fprintf(stderr, "%s:%d: error: More than one value in 'create_single_rvalue_node', this has not been delt with yet\nThe tokens are:\n", __FILE__, __LINE__);
        for (int i = start; i < end + 1; i++) {
            fprintf(stderr, "    %s\n", ((token**)tokens->data)[i]->data);
        }
    }

    token* t = ((token**)tokens->data)[start];
    AST_node* node = malloc(sizeof(AST_node));
    init_AST_node(node);
    // TODO: Better type handling and stuff
    if (token_is_number(t)) {
        node->node_type = CONSTANT;
        node->token = t;
        constant* c = malloc(sizeof(constant));
        
        // TODO: Type stuff
        // c->type =
        c->value = t->data;
        node->data = (void*)c;
    } else {
        if (!is_varible_defined(scope, t->data)) {
            fprintf(stderr, "%s:%d: error: Varible '%s' on posistion %i is not defined\n", __FILE__, __LINE__, t->data, t->pos_in_file);
        }
        //node = get_node_from_name(scope, t->data);
        // TODO: For now we assume that it is a varible, if not a number
        node->node_type = VARIBLE;
        node->token = t;
        varible* v = malloc(sizeof(varible));
        
        // TODO: Type stuff
        // v->type =
        v->name = t->data;
        node->data = (void*)v;
        
    }
    return node;

}

void generate_function_call_inputs(AST_node* scope, AST_node* function_node, dynamic_array* tokens, int start, int end)
{
    start++;
    end--;

    // add(add(a, 2), 3);
    for (int i = start; i < end + 1; i++) {
        int end_arg = find_comma(tokens, i, end);
        AST_node* node = create_expression_node(scope, tokens, i, end_arg);
        da_append(function_node->children, node, AST_node*);
        i = end_arg;
    }
}

AST_node* create_function_call_node(AST_node* scope, dynamic_array* tokens, int start, int end)
{
    // add(a, b)
    // [add] [(] [a] [,] [b] [)]
    //  ^start                ^end
    AST_node* function_call_node = malloc(sizeof(AST_node));
    init_AST_node(function_call_node);
    function_call_node->node_type = FUNCTION_CALL;
    function_call_node->token = ((token**)tokens->data)[start];

    function_call* fc = malloc(sizeof(function));
    // TODO: type stuff
    //fc->return_type = 
    fc->name = function_call_node->token->data;
    function_call_node->data = (void*)fc;

    generate_function_call_inputs(scope, function_call_node, tokens, start + 1, end);
    return function_call_node;
    
}

AST_node* create_expression_node(AST_node* scope, dynamic_array* tokens, int start, int end)
{

    while (((token**)tokens->data)[end]->type == SEMICOLON) {
        end--;
    }
    while (((token**)tokens->data)[start]->type == PAREN_OPEN) {
        start++;
        end--;
    }

    for (int i = sizeof(operator_mapping)/sizeof(operator_mapping[0]); i >= 0; i--) {
        for (int j = start; j < end + 1; j++) {

            // Skip the things in parentheses
            if (((token**)tokens->data)[j]->type == PAREN_OPEN) {
                j = get_closing_paren_location(tokens, j);
            }
            // Go from lowest precedence to highest and create left/right nodes containing the left and right of the expression
            else if (token_is_operator(((token**)tokens->data)[j]) && ((token**)tokens->data)[j]->type == operator_mapping[i].type) {
                AST_node* node = malloc(sizeof(AST_node));
                init_AST_node(node);
                node->node_type = OPERATOR;
                node->token = ((token**)tokens->data)[j];
                operator* o = malloc(sizeof(operator));
                o->type = node->token->type;
                o->name = node->token->data;
                node->data = (void*)o; 

                int left_start = start;
                int left_end = j - 1;
                AST_node* left_node = create_expression_node(scope, tokens, left_start, left_end);
                da_append(node->children, left_node, AST_node*);
                
                if (o->type == POST_INCREMENT || o->type == POST_DECREMENT) {
                } else {
                    int right_start = j + 1;
                    int right_end = end;
                    AST_node* right_node = create_expression_node(scope, tokens, right_start, right_end);
                    da_append(node->children, right_node, AST_node*);
                }
                
                return node;
            }
        }
    }

        // Check for function calls
    for (int i = start; i < end + 1; i++) {
        if (((token**)tokens->data)[i + 0]->type == OTHER &&
            ((token**)tokens->data)[i + 1]->type == PAREN_OPEN) {
            int function_call_start = i;
            int function_call_end = get_closing_paren_location(tokens, i + 1);
            return create_function_call_node(scope, tokens, function_call_start, function_call_end);
            
        }
    }

    // There are no operators in the currnet token list
    // We have reached a constant number, string, char, etc. Now we must make and return it.
    //printf("start loc: %i, end loc: %i\n", start, end);
    return create_constant_node(scope, tokens, start, end);
}

void create_varible_node(AST_node* scope, AST_node* node, dynamic_array* tokens, token* t)
{
    int start = get_token_location(tokens, t);
    int end = find_semi_colon(tokens, start);

    // Equals
    AST_node* equals_node = malloc(sizeof(AST_node));
    init_AST_node(equals_node);
            
    equals_node->token = ((token**)tokens->data)[start + 1];
    equals_node->node_type = OPERATOR;
            
    operator* equals = malloc(sizeof(operator));
    equals->type = ((token**)tokens->data)[start + 1]->type;
    equals->name = ((token**)tokens->data)[start + 1]->data;
    equals_node->data = (void*)equals;

    // Varible
    AST_node* varible_node = malloc(sizeof(AST_node));
    init_AST_node(varible_node);
            
    varible_node->token = ((token**)tokens->data)[start];
    varible_node->node_type = VARIBLE;
            
    varible* varible = malloc(sizeof(varible));
    // TODO: types
    varible->name = ((token**)tokens->data)[start]->data;
    varible_node->data = (void*)varible;

    // Expression
    AST_node* expression_node = create_expression_node(scope, tokens, start + 2, end - 1);

    // Adding nodes
    da_append(equals_node->children, varible_node, AST_node*);
    da_append(equals_node->children, expression_node, AST_node*);
    da_append(node->children, equals_node, AST_node*);
    
}

void create_return_node(AST_node* scope, AST_node* node, dynamic_array* tokens, token* t)
{
    AST_node* return_node = malloc(sizeof(AST_node));
    init_AST_node(return_node);
    
    return_node->node_type = KEY_WORD;
    return_node->token = t;

    key_word* kw = malloc(sizeof(key_word));
    kw->name = t->data;
    kw->key_word_type = RETURN;
    kw->data = NULL;
    return_node->data = kw;

    // Create expression
    int start = get_token_location(tokens, t) + 1;
    int end = find_semi_colon(tokens, start);
    AST_node* return_expression_node = create_expression_node(scope, tokens, start, end);

    da_append(return_node->children, return_expression_node, AST_node*);
    da_append(node->children, return_node, AST_node*);

}

int create_if_node(AST_node* scope, AST_node* node, dynamic_array* tokens, token* t)
{
    AST_node* if_node = malloc(sizeof(AST_node));
    init_AST_node(if_node);
    
    if_node->node_type = KEY_WORD;
    if_node->token = t;

    key_word* kw = malloc(sizeof(key_word));
    kw->name = t->data;
    kw->key_word_type = IF;
    kw->data = NULL;
    if_node->data = kw;

    int expression_start = get_token_location(tokens, t) + 1;
    int expression_end = get_closing_paren_location(tokens, expression_start);
    AST_node* if_expression_node = create_expression_node(scope, tokens, expression_start, expression_end);
    da_append(if_node->children, if_expression_node, AST_node*);
    
    int block_start = expression_end + 1;
    int block_end = get_closing_paren_location(tokens, block_start);
    generate_AST(scope, if_node, tokens, block_start, block_end);

    da_append(node->children, if_node, AST_node*);
    if (((token**)tokens->data)[block_end + 1]->type == ELSE) {
         return create_else_node(scope, if_node, tokens, ((token**)tokens->data)[block_end + 1]);
    }

    
    return block_end;
}

int create_else_node(AST_node* scope, AST_node* node, dynamic_array* tokens, token* t)
{
    AST_node* else_node = malloc(sizeof(AST_node));
    init_AST_node(else_node);
    
    
    else_node->node_type = KEY_WORD;
    else_node->token = t;

    key_word* kw = malloc(sizeof(key_word));
    kw->name = t->data;
    kw->key_word_type = ELSE;
    kw->data = NULL;
    else_node->data = kw;

    da_append(node->children, else_node, AST_node*);
    
    int else_location = get_token_location(tokens, t);
    if (((token**)tokens->data)[else_location + 1]->type == IF) {
        return create_if_node(scope, else_node, tokens, ((token**)tokens->data)[else_location + 1]);
    }

    int block_start = else_location + 1;
    int block_end = get_closing_paren_location(tokens, block_start);
    generate_AST(scope, else_node, tokens, block_start, block_end);
    
    return block_end;
}

int create_for_node(AST_node* scope, AST_node* node, dynamic_array* tokens, token* t)
{
    AST_node* for_node = malloc(sizeof(AST_node));
    init_AST_node(for_node);
    
    for_node->node_type = KEY_WORD;
    for_node->token = t;

    key_word* kw = malloc(sizeof(key_word));
    kw->name = t->data;
    kw->key_word_type = FOR;
    kw->data = NULL;
    for_node->data = kw;
    da_append(node->children, for_node, AST_node*);

    int for_loop_start = get_token_location(tokens, t) + 1;
    int for_loop_end = get_closing_paren_location(tokens, for_loop_start);

    int for_loop_prelude_start = for_loop_start + 1;
    int for_loop_prelude_end = find_semi_colon(tokens, for_loop_prelude_start);
    generate_AST(scope, for_node, tokens, for_loop_prelude_start, for_loop_prelude_end);

    int for_loop_condition_start = for_loop_prelude_end + 1;
    int for_loop_condition_end = find_semi_colon(tokens, for_loop_condition_start);

    AST_node* condition_node = create_expression_node(scope, tokens, for_loop_condition_start, for_loop_condition_end);
    da_append(for_node->children, condition_node, AST_node*);
    
    int for_loop_epilogue_start = for_loop_condition_end;
    int for_loop_epilogue_end = for_loop_end + 1;
    generate_AST(scope, for_node, tokens, for_loop_epilogue_start, for_loop_epilogue_end);

    int block_start = for_loop_end + 1;
    int block_end = get_closing_paren_location(tokens, block_start);
    generate_AST(scope, for_node, tokens, block_start, block_end);
    
    return block_end;
}

int create_while_node(AST_node* scope, AST_node* node, dynamic_array* tokens, token* t)
{
    AST_node* while_node = malloc(sizeof(AST_node));
    init_AST_node(while_node);
    
    while_node->node_type = KEY_WORD;
    while_node->token = t;

    key_word* kw = malloc(sizeof(key_word));
    kw->name = t->data;
    kw->key_word_type = WHILE;
    kw->data = NULL;
    while_node->data = kw;
    da_append(node->children, while_node, AST_node*);

    int expression_start = get_token_location(tokens, t) + 1;
    int expression_end = get_closing_paren_location(tokens, expression_start);
    AST_node* while_expression_node = create_expression_node(scope, tokens, expression_start, expression_end);
    da_append(while_node->children, while_expression_node, AST_node*);

    int block_start = expression_end + 1;
    int block_end = get_closing_paren_location(tokens, block_start);
    generate_AST(scope, while_node, tokens, block_start, block_end);
    return block_end;
}


int create_key_word_node(AST_node* scope, AST_node* node, dynamic_array* tokens, token* t)
{
    switch (t->type) {
    case RETURN: {
        create_return_node(scope, node, tokens, t);
        return find_semi_colon(tokens, get_token_location(tokens, t));
        break;
    }
    case IF: {
        return create_if_node(scope, node, tokens, t);
        break;
    }
    case FOR: {
        return create_for_node(scope, node, tokens, t);
        break;
    }
    case WHILE: {
        return create_while_node(scope, node, tokens, t);
        break;
    }
    default: {
        fprintf(stderr, "%s:%d: TODO: Key word %s %i was not handled\n", __FILE__, __LINE__, t->data, t->pos_in_file);
        break;
    }
    }
    fprintf(stderr, "%s:%d: error: UNREACHABLE\n", __FILE__, __LINE__);
    return -1;
}

int match_tokens(AST_node* scope, AST_node* node, dynamic_array* tokens, dynamic_array* token_stack)
{

    if (token_stack->count == 1) {
        token* t1 = ((token**)token_stack->data)[0];
        // for (...) {...}, while(...) {...}, if (...) {...}
        // All keywords should fit here
        if (token_is_key_word(t1)) {
            return create_key_word_node(scope, node, tokens, t1);
        }
    }
    if (token_stack->count == 2) {
        token* t1 = ((token**)token_stack->data)[0];
        token* t2 = ((token**)token_stack->data)[1];
        if (t1->type == OTHER && t2->type == EQUALS) {
            // i = ...;
            // Modify varible

            create_varible_node(scope, node, tokens, t1);
            return find_semi_colon(tokens, get_token_location(tokens, t1));
        }

    }
    if (token_stack->count == 3) {
        token* t1 = ((token**)token_stack->data)[0];
        token* t2 = ((token**)token_stack->data)[1];
        token* t3 = ((token**)token_stack->data)[2];
        if (t1->type == OTHER && t2->type == OTHER && t3->type == EQUALS) {
            // int i = ...;
            // Initalize varible
            create_varible_node(scope, node, tokens, t2);
            return find_semi_colon(tokens, get_token_location(tokens, t1));
        }
    }

    if (((token**)token_stack->data)[token_stack->count - 1]->type == SEMICOLON ||
        ((token**)token_stack->data)[token_stack->count - 1]->type == PAREN_CLOSE) {
        // i++;
        // for just expressions
        token* t1 = ((token**)token_stack->data)[0];
        token* tend = ((token**)token_stack->data)[token_stack->count - 1];
        
        int start = get_token_location(tokens, t1);
        int end = get_token_location(tokens, tend);
        AST_node* expression_node = create_expression_node(scope, tokens, start, end);
        da_append(node->children, expression_node, AST_node*);
        
        return end + 1;
    }
    return -1;
}

void generate_AST(AST_node* scope, AST_node* node, dynamic_array* tokens, int start, int end)
{
    start++;
    end--;

    dynamic_array token_stack;
    da_init(&token_stack, token);

    for (int i = start; i < end + 1; i++) {
        da_append(&token_stack, ((token**)tokens->data)[i], token*);
        int new_location = match_tokens(scope, node, tokens, &token_stack);
        if (new_location != -1) {
            i = new_location;
            token_stack.count = 0;
        }
    }
    
}

void generate_function_inputs(dynamic_array* tokens, AST_node* node, int start, int end)
{
    start++;
    end--;

    for (int i = start; i < end + 1; i += 3) {
        AST_node* fi = malloc(sizeof(AST_node));
        init_AST_node(fi);
        
        fi->token = ((token**)tokens->data)[i + 1];
        fi->node_type = VARIBLE;
            
        varible* varible = malloc(sizeof(varible));
        // TODO: types
        varible->name = ((token**)tokens->data)[i + 1]->data;
        fi->data = (void*)varible;
        
        da_append(node->children, fi, AST_node*);
    }
}

AST_node* create_function_node(dynamic_array* tokens, int location) {
    // int foo(int a, float b)
    // ^location
    //token* return_type = ((token**)tokens->data)[location];
    token* function_name = ((token**)tokens->data)[location + 1];

    AST_node* function_node = malloc(sizeof(AST_node));
    init_AST_node(function_node);
    function* f = malloc(sizeof(function));
    // TODO: type stuff
    //main_function->return_type = get_type_from_str(((token**)tokens->data)[location]->data);
    f->name = function_name->data;
    
    function_node->token = function_name;
    function_node->node_type = FUNCTION;
    function_node->data = (void*)f;

    int inputs_start = location + 2;
    int inputs_end = get_closing_paren_location(tokens, inputs_start);
    
    AST_node* function_inputs_node = malloc(sizeof(AST_node));
    init_AST_node(function_inputs_node);
    function_inputs_node->node_type = FUNCTION_INPUT;
    function_inputs_node->token = function_name;
    
    generate_function_inputs(tokens, function_inputs_node, inputs_start, inputs_end);
    da_append(function_node->children, function_inputs_node, AST_node*);
    
    
    return function_node;
    
}

void generate_functions(dynamic_array* functions, dynamic_array* tokens)
{
    dynamic_array token_stack;
    da_init(&token_stack, token);
    for (int i = 0; i < tokens->count; i++) {
        da_append(&token_stack, ((token**)tokens->data)[i], token*);
        if (token_stack.count == 3) {
            token* t1 = ((token**)token_stack.data)[0];
            token* t2 = ((token**)token_stack.data)[1];
            token* t3 = ((token**)token_stack.data)[2];
            if (t1->type == OTHER && t2->type == OTHER && t3->type == PAREN_OPEN) {
                int function_location = get_token_location(tokens, t1);
                AST_node* function_node = create_function_node(tokens, function_location);
                da_append(functions, function_node, AST_node*);

                int function_start = get_closing_paren_location(tokens, get_token_location(tokens, t3)) + 1;
                int function_end = get_closing_paren_location(tokens, function_start) ;
                generate_AST(function_node, function_node, tokens, function_start, function_end);
                
                i = function_end;
                token_stack.count = 0;

            }
        }
    }

}

void update_varible_stack_posistion(AST_node* node, char* varible_name, int stack_pos)
{
    if (node->node_type == VARIBLE) {
        if (strcmp(((varible*)node->data)->name, varible_name) == 0) {
            ((varible*)node->data)->stack_pos = stack_pos;
        }
    }
    for (int i = 0; i < node->children->count; i++) {
        AST_node* child = ((AST_node**)node->children->data)[i];
        update_varible_stack_posistion(child, varible_name, stack_pos);
    }
}

int generate_stack_posistions(AST_node* scope, AST_node* node , int stack_size)
{
    for (int i = 0; i < node->children->count; i++) {
        AST_node* child = ((AST_node**)node->children->data)[i];
        if (child->node_type == VARIBLE) {
            if (((varible*)child->data)->stack_pos == 0) {
                // TOOD: Deal with different types / different sizes
                stack_size += 4;
                update_varible_stack_posistion(scope, ((varible*)child->data)->name, stack_size);
            }
        }
        stack_size = generate_stack_posistions(scope, child, stack_size);
    }
    
    return stack_size;
    
}
