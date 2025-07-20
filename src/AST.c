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


AST_node* create_constant_node(AST_node* scope, context* ctx, int start, int end)
{
    while (((token**)ctx->tokens->data)[start]->type == PAREN_OPEN && ((token**)ctx->tokens->data)[end + 1]->type == PAREN_CLOSE) {
        start++;
        end--;
    }
        
    if (end - start > 1) {
        fprintf(stderr, "%s:%d: error: More than one value in 'create_constant_node', this has not been delt with yet\nThe tokens are:\n", __FILE__, __LINE__);
        for (int i = start; i < end + 1; i++) {
            fprintf(stderr, "    %s\n", ((token**)ctx->tokens->data)[i]->data);
        }
        *(int*)0 = 0;
    }
    
    

    token* t = ((token**)ctx->tokens->data)[start];
    AST_node* node = malloc(sizeof(AST_node));
    init_AST_node(node);
    // TODO: Better type handling and stuff
    if (token_is_number(t) || token_is_string(t)) {
        node->node_type = CONSTANT;
        node->token = t;
        constant* c = malloc(sizeof(constant));
        
        // TODO: More type stuff
        if (token_is_number(t)) {
            c->type = get_number_type(ctx->types, t);
        } else if (token_is_string(t)) {
            c->type = get_string_type(ctx->types);
        }
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
        
        AST_node* varible_definition = get_node_from_name(scope, t->data);

        v->type = ((varible*)varible_definition->data)->type;
        v->name = t->data;
        node->data = (void*)v;
        
    }
    return node;

}

void generate_function_call_inputs(AST_node* scope, AST_node* function_node, context* ctx, int start, int end)
{
    start++;
    end--;

    // add(add(a, 2), 3);
    for (int i = start; i < end + 1; i++) {
        int end_arg = find_comma(ctx->tokens, i, end);
        AST_node* node = create_expression_node(scope, ctx, i, end_arg);
        da_append(function_node->children, node, AST_node*);
        i = end_arg;
    }
}

AST_node* create_function_call_node(AST_node* scope, context* ctx, int start, int end)
{
    // add(a, b)
    // [add] [(] [a] [,] [b] [)]
    //  ^start                ^end
    AST_node* function_call_node = malloc(sizeof(AST_node));
    init_AST_node(function_call_node);
    function_call_node->node_type = FUNCTION_CALL;
    function_call_node->token = ((token**)ctx->tokens->data)[start];
    
    function_call* fc = malloc(sizeof(function));
    fc->name = function_call_node->token->data;
    for (int i = 0; i < ctx->functions->count; i++) {
        AST_node* f = ((AST_node**)ctx->functions->data)[i];
        if (strcmp(((function*)f->data)->name, fc->name) == 0) {
            fc->return_type = ((function*)f->data)->return_type;
        }
    }


    function_call_node->data = (void*)fc;

    generate_function_call_inputs(scope, function_call_node, ctx, start + 1, end);
    return function_call_node;
    
}
AST_node* create_access_node(AST_node* scope, context* ctx, int start, int end)
{
    // Two cases:
    // 1:
    // arr[i + number - 3]
    // ^start            ^ end
    // 2:
    // (arr->data())[i + number - 3]
    // ^start       ^access_start  ^end
    
    AST_node* access_node = malloc(sizeof(AST_node));
    init_AST_node(access_node);
    access_node->node_type = ACCESS;
    access_node->token = ((token**)ctx->tokens->data)[start];

    
    int access_start = -1;
    if (((token**)ctx->tokens->data)[start]->type == PAREN_OPEN) {
        access_start = get_closing_paren_location(ctx->tokens, start) + 1;
    } else if (((token**)ctx->tokens->data)[start]->type == OTHER) {
        access_start = start + 1;
    } else {
        fprintf(stderr, "%s:%d: error: Unhandled and maybe unreachable case: token %s at %i\n", __FILE__, __LINE__, ((token**)ctx->tokens->data)[start]->data, ((token**)ctx->tokens->data)[start]->pos_in_file);
    }

    AST_node* to_access_node = create_expression_node(scope, ctx, start, access_start - 1);

    AST_node* access_point_node = create_expression_node(scope, ctx, access_start + 1, end - 1);

    
    da_append(access_node->children, to_access_node, AST_node*);
    da_append(access_node->children, access_point_node, AST_node*);
    return access_node;
    
}

type* get_type_from_node(dynamic_array* types, AST_node* node)
{
    if (node->node_type == VARIBLE) {
        //*(int*)0 = 0;
        return (((varible*)node->data)->type);
    }
    if (node->node_type == CONSTANT) {
        //*(int*)0 = 0;
        return (((constant*)node->data)->type);
    }
    if (node->node_type == ACCESS) {
        AST_node* n = ((AST_node**)node->children->data)[0];
        type* t = get_type_from_node(types, n);
        //*(int*)0 = 0;
        return get_dereferenced_type(types, t);
    }
    for (int i = 0; i < node->children->count; i++) {
        type* t = get_type_from_node(types, ((AST_node**)node->children->data)[i]);
        if (t != NULL) {
            return t;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find type from node with token %s at %i\n", __FILE__, __LINE__, node->token->data, node->token->pos_in_file);
    return NULL;
}

AST_node* create_cast_node(AST_node* scope, context* ctx, int start, int end)
{
    // (int*)arr[i]
    // ^start     ^end
    
    int cast_start = start;
    int cast_end = get_closing_paren_location(ctx->tokens, cast_start);
    type* ty = get_type(ctx->tokens, ctx->types, ((token**)ctx->tokens->data)[cast_start + 1]);

    AST_node* cast_expr = create_expression_node(scope, ctx, cast_end + 1, end);
    
    AST_node* cast_node = malloc(sizeof(AST_node));
    init_AST_node(cast_node);
    cast_node->node_type = CAST;
    cast_node->token = ((token**)ctx->tokens->data)[cast_start + 1];
    
    cast* c = malloc(sizeof(cast));
    c->from_type = get_type_from_node(ctx->types, cast_expr);
    c->to_type = ty;
    //*(int*)0 = 0;

    cast_node->data = (void*)c;

    da_append(cast_node->children, cast_expr, AST_node*);
    return cast_node;
    
}

AST_node* create_expression_node(AST_node* scope, context* ctx, int start, int end)
{

    while (((token**)ctx->tokens->data)[end]->type == SEMICOLON) {
        end--;
    }
    while (((token**)ctx->tokens->data)[start]->type == PAREN_OPEN && ((token**)ctx->tokens->data)[end]->type == PAREN_CLOSE) {
        start++;
        end--;
    }

    

    for (int i = sizeof(operator_mapping)/sizeof(operator_mapping[0]); i >= 0; i--) {
        for (int j = start; j < end + 1; j++) {

            // Skip the things in parentheses
            if (((token**)ctx->tokens->data)[j]->type == PAREN_OPEN) {
                j = get_closing_paren_location(ctx->tokens, j);
            }
            // Go from lowest precedence to highest and create left/right nodes containing the left and right of the expression
            else if (token_is_operator(((token**)ctx->tokens->data)[j]) && ((token**)ctx->tokens->data)[j]->type == operator_mapping[i].type) {
                AST_node* node = malloc(sizeof(AST_node));
                init_AST_node(node);
                node->node_type = OPERATOR;
                node->token = ((token**)ctx->tokens->data)[j];
                operator* o = malloc(sizeof(operator));
                o->type = node->token->type;
                o->name = node->token->data;
                node->data = (void*)o; 

                int left_start = start;
                int left_end = j - 1;
                AST_node* left_node = create_expression_node(scope, ctx, left_start, left_end);
                da_append(node->children, left_node, AST_node*);
                
                if (o->type == POST_INCREMENT || o->type == POST_DECREMENT) {
                } else {
                    int right_start = j + 1;
                    int right_end = end;
                    AST_node* right_node = create_expression_node(scope, ctx, right_start, right_end);
                    da_append(node->children, right_node, AST_node*);
                }
                
                return node;
            }
        }
    }

    // Check for function calls, acessors and casting
    for (int i = start; i < end + 1; i++) {

        if (((token**)ctx->tokens->data)[i + 0]->type == OTHER &&
            ((token**)ctx->tokens->data)[i + 1]->type == PAREN_OPEN) {
            int function_call_start = i;
            int function_call_end = get_closing_paren_location(ctx->tokens, i + 1);
            return create_function_call_node(scope, ctx, function_call_start, function_call_end);
            
        }
        else if (((token**)ctx->tokens->data)[i + 0]->type == PAREN_OPEN &&
                 ((token**)ctx->tokens->data)[i + 1]->type == TYPE) {
            return create_cast_node(scope, ctx, start, end);
        }
        else if (((token**)ctx->tokens->data)[i]->type == PAREN_SQUARE_OPEN) {
            return create_access_node(scope, ctx, start, end);
        }
    }

    // There are no operators in the currnet token list
    // We have reached a constant number, string, char, etc. Now we must make and return it.
    //printf("start loc: %i, end loc: %i\n", start, end);
    return create_constant_node(scope, ctx, start, end);
}


     

void create_varible_node(AST_node* scope, AST_node* node, context* ctx, token* t)
{
    // char * i = ...
    //        ^t
    int start = get_token_location(ctx->tokens, t);
    int end = find_semi_colon(ctx->tokens, start);

    // Equals
    AST_node* equals_node = malloc(sizeof(AST_node));
    init_AST_node(equals_node);
            
    equals_node->token = ((token**)ctx->tokens->data)[start + 1];
    equals_node->node_type = OPERATOR;
            
    operator* equals = malloc(sizeof(operator));
    equals->type = ((token**)ctx->tokens->data)[start + 1]->type;
    equals->name = ((token**)ctx->tokens->data)[start + 1]->data;
    equals_node->data = (void*)equals;

    // Varible
    AST_node* varible_node = malloc(sizeof(AST_node));
    init_AST_node(varible_node);
            
    varible_node->token = ((token**)ctx->tokens->data)[start];
    varible_node->node_type = VARIBLE;
            
    varible* v = malloc(sizeof(varible));

    if (((token**)ctx->tokens->data)[start - 1]->type == TIMES || ((token**)ctx->tokens->data)[start - 1]->type == TYPE) {
        int i = 1;
        while (((token**)ctx->tokens->data)[start - i]->type == TIMES) {
            i++;
        }
        v->type = get_type(ctx->tokens, ctx->types, ((token**)ctx->tokens->data)[start - i]);
    } else {
        AST_node* definition_node = get_node_from_name(scope, varible_node->token->data);
        v->type = ((varible*)definition_node->data)->type;
    }

    //*(int*)0 = 0;
    
    v->name = ((token**)ctx->tokens->data)[start]->data;
    varible_node->data = (void*)v;

    // Expression
    AST_node* expression_node = create_expression_node(scope, ctx, start + 2, end - 1);

    // Adding nodes
    da_append(equals_node->children, varible_node, AST_node*);
    da_append(equals_node->children, expression_node, AST_node*);
    da_append(node->children, equals_node, AST_node*);
    
}

void create_return_node(AST_node* scope, AST_node* node, context* ctx, token* t)
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
    int start = get_token_location(ctx->tokens, t) + 1;
    int end = find_semi_colon(ctx->tokens, start);
    AST_node* return_expression_node = create_expression_node(scope, ctx, start, end);

    da_append(return_node->children, return_expression_node, AST_node*);
    da_append(node->children, return_node, AST_node*);

}

int create_if_node(AST_node* scope, AST_node* node, context* ctx, token* t)
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

    int expression_start = get_token_location(ctx->tokens, t) + 1;
    int expression_end = get_closing_paren_location(ctx->tokens, expression_start);
    AST_node* if_expression_node = create_expression_node(scope, ctx, expression_start, expression_end);
    da_append(if_node->children, if_expression_node, AST_node*);

    int block_start;
    int block_end;
    if (token_is_parentheses(((token**)ctx->tokens->data)[expression_end + 1])) {
        block_start = expression_end + 1;
        block_end = get_closing_paren_location(ctx->tokens, block_start);
        generate_AST(scope, if_node, ctx, block_start, block_end);
    } else {
        block_start = expression_end;
        block_end = find_semi_colon(ctx->tokens, block_start);
        generate_AST(scope, if_node, ctx, block_start, block_end);
    }

    da_append(node->children, if_node, AST_node*);
    if (((token**)ctx->tokens->data)[block_end + 1]->type == ELSE) {
        return create_else_node(scope, if_node, ctx, ((token**)ctx->tokens->data)[block_end + 1]);
    }

    
    return block_end;
}

int create_else_node(AST_node* scope, AST_node* node, context* ctx, token* t)
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
    
    int else_location = get_token_location(ctx->tokens, t);
    if (((token**)ctx->tokens->data)[else_location + 1]->type == IF) {
        return create_if_node(scope, else_node, ctx, ((token**)ctx->tokens->data)[else_location + 1]);
    }

    int block_start = else_location + 1;
    int block_end = get_closing_paren_location(ctx->tokens, block_start);
    generate_AST(scope, else_node, ctx, block_start, block_end);
    
    return block_end;
}

int create_for_node(AST_node* scope, AST_node* node, context* ctx, token* t)
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

    int for_loop_start = get_token_location(ctx->tokens, t) + 1;
    int for_loop_end = get_closing_paren_location(ctx->tokens, for_loop_start);

    int for_loop_prelude_start = for_loop_start + 1;
    int for_loop_prelude_end = find_semi_colon(ctx->tokens, for_loop_prelude_start);
    generate_AST(scope, for_node, ctx, for_loop_prelude_start, for_loop_prelude_end);

    int for_loop_condition_start = for_loop_prelude_end + 1;
    int for_loop_condition_end = find_semi_colon(ctx->tokens, for_loop_condition_start);

    AST_node* condition_node = create_expression_node(scope, ctx, for_loop_condition_start, for_loop_condition_end);
    da_append(for_node->children, condition_node, AST_node*);
    
    int for_loop_epilogue_start = for_loop_condition_end;
    int for_loop_epilogue_end = for_loop_end + 1;
    generate_AST(scope, for_node, ctx, for_loop_epilogue_start, for_loop_epilogue_end);

    int block_start = for_loop_end + 1;
    int block_end = get_closing_paren_location(ctx->tokens, block_start);
    
    generate_AST(scope, for_node, ctx, block_start, block_end);
    
    return block_end;
}

int create_while_node(AST_node* scope, AST_node* node, context* ctx, token* t)
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

    int expression_start = get_token_location(ctx->tokens, t) + 1;
    int expression_end = get_closing_paren_location(ctx->tokens, expression_start);
    AST_node* while_expression_node = create_expression_node(scope, ctx, expression_start, expression_end);
    da_append(while_node->children, while_expression_node, AST_node*);

    int block_start = expression_end + 1;
    int block_end = get_closing_paren_location(ctx->tokens, block_start);
    generate_AST(scope, while_node, ctx, block_start, block_end);
    return block_end;
}


int create_key_word_node(AST_node* scope, AST_node* node, context* ctx, token* t)
{
    switch (t->type) {
    case RETURN: {
        create_return_node(scope, node, ctx, t);
        return find_semi_colon(ctx->tokens, get_token_location(ctx->tokens, t));
        break;
    }
    case IF: {
        return create_if_node(scope, node, ctx, t);
        break;
    }
    case FOR: {
        return create_for_node(scope, node, ctx, t);
        break;
    }
    case WHILE: {
        return create_while_node(scope, node, ctx, t);
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

int match_tokens(AST_node* scope, AST_node* node, context* ctx, dynamic_array* token_stack)
{
    
    if (token_stack->count == 1) {
        token* t1 = ((token**)token_stack->data)[0];
        // for (...) {...}, while(...) {...}, if (...) {...}
        // All keywords should fit here
        if (token_is_key_word(t1)) {
            return create_key_word_node(scope, node, ctx, t1);
        }
    } else
        
    if (token_stack->count == 2) {
        token* t1 = ((token**)token_stack->data)[0];
        token* t2 = ((token**)token_stack->data)[1];
        if (t1->type == OTHER && token_is_modifier(t2)) {
            // i = ...;
            // Modify varible

            create_varible_node(scope, node, ctx, t1);
            return find_semi_colon(ctx->tokens, get_token_location(ctx->tokens, t1));
        }

    } else
    
    if (token_stack->count >= 3) {
        token* t1 = ((token**)token_stack->data)[0];
        token* t2 = ((token**)token_stack->data)[token_stack->count - 2];
        token* t3 = ((token**)token_stack->data)[token_stack->count - 1];
        if (t1->type == TYPE && t2->type == OTHER && t3->type == EQUALS) {
            // char * i = ...;
            // Initalize varible
            create_varible_node(scope, node, ctx, t2);
            return find_semi_colon(ctx->tokens, get_token_location(ctx->tokens, t1));
        }
    } 

    if (((token**)token_stack->data)[token_stack->count - 1]->type == SEMICOLON ||
        ((token**)token_stack->data)[token_stack->count - 1]->type == PAREN_CLOSE) {
        // i++;
        // for just expressions

        token* t1 = ((token**)token_stack->data)[0];
        token* tend = ((token**)token_stack->data)[token_stack->count - 1];
        
        int start = get_token_location(ctx->tokens, t1);
        int end = get_token_location(ctx->tokens, tend);
        AST_node* expression_node = create_expression_node(scope, ctx, start, end);
        da_append(node->children, expression_node, AST_node*);
        
        return end + 1;
    }
    return -1;
}

void generate_AST(AST_node* scope, AST_node* node, context* ctx, int start, int end)
{
    start++;
    end--;
    

    dynamic_array token_stack;
    da_init(&token_stack, token);

    for (int i = start; i < end + 1; i++) {
        da_append(&token_stack, ((token**)ctx->tokens->data)[i], token*);
        int new_location = match_tokens(scope, node, ctx, &token_stack);
        if (new_location != -1) {
            i = new_location;
            token_stack.count = 0;
        }
    }
    
}

void generate_function_inputs(context* ctx, AST_node* node, int start, int end)
{
    start++;
    end--;

    for (int i = start; i < end + 1;) {
        AST_node* fi = malloc(sizeof(AST_node));
        init_AST_node(fi);
        
        
        fi->node_type = VARIBLE;
            
        varible* varible = malloc(sizeof(varible));
        varible->type = get_type(ctx->tokens, ctx->types, ((token**)ctx->tokens->data)[i]);
        

        fi->token = ((token**)ctx->tokens->data)[i + varible->type->ptr_count + 1];
        varible->name = ((token**)ctx->tokens->data)[i + varible->type->ptr_count + 1]->data;
        fi->data = (void*)varible;
        
        da_append(node->children, fi, AST_node*);

        i += varible->type->ptr_count + 3;
    }
}

AST_node* create_function_node(context* ctx, int location)
{
    // [int] [*] [foo] [(] [int] [a] [,] [float] [b] [)]
    // ^location
    type* return_type = get_type(ctx->tokens, ctx->types, ((token**)ctx->tokens->data)[location]);
    token* function_name = ((token**)ctx->tokens->data)[location + return_type->ptr_count + 1];

    AST_node* function_node = malloc(sizeof(AST_node));
    init_AST_node(function_node);
    function* f = malloc(sizeof(function));
    // TODO: type stuff
    f->return_type = return_type;
    f->name = function_name->data;
    
    function_node->token = function_name;
    function_node->node_type = FUNCTION;
    function_node->data = (void*)f;

    int inputs_start = location + return_type->ptr_count + 2;
    int inputs_end = get_closing_paren_location(ctx->tokens, inputs_start);
    
    AST_node* function_inputs_node = malloc(sizeof(AST_node));
    init_AST_node(function_inputs_node);
    function_inputs_node->node_type = FUNCTION_INPUT;
    function_inputs_node->token = function_name;
    
    generate_function_inputs(ctx, function_inputs_node, inputs_start, inputs_end);
    da_append(function_node->children, function_inputs_node, AST_node*);
    
    return function_node;
    
}


bool is_function_definition(context* ctx, dynamic_array* token_stack)
{
    if (token_stack->count >= 3) {
        token* t1 = ((token**)token_stack->data)[0];
        type* t = get_type(ctx->tokens, ctx->types, t1);

        if (token_stack->count < t->ptr_count + 2) {
            return false;
        }
        
        token* t2 = ((token**)token_stack->data)[1 + t->ptr_count];
        token* t3 = ((token**)token_stack->data)[2 + t->ptr_count];
        if (t1->type == TYPE && t2->type == OTHER && t3->type == PAREN_OPEN) {
            return true;
        }
    }
    return false;
}

void generate_functions(context* ctx)
{
    dynamic_array token_stack;
    da_init(&token_stack, token);
    for (int i = 0; i < ctx->tokens->count; i++) {
        da_append(&token_stack, ((token**)ctx->tokens->data)[i], token*);
        if (is_function_definition(ctx, &token_stack)) {
            token* t1 = ((token**)token_stack.data)[0];
            token* t3 = ((token**)token_stack.data)[token_stack.count - 1];

            int function_location = get_token_location(ctx->tokens, t1);
            AST_node* function_node = create_function_node(ctx, function_location);
            da_append(ctx->functions, function_node, AST_node*);

            int function_start = get_closing_paren_location(ctx->tokens, get_token_location(ctx->tokens, t3)) + 1;
            int function_end = get_closing_paren_location(ctx->tokens, function_start) ;
            generate_AST(function_node, function_node, ctx, function_start, function_end);
                
            i = function_end;
            token_stack.count = 0;

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
                stack_size += ((varible*)child->data)->type->size;
                update_varible_stack_posistion(scope, ((varible*)child->data)->name, stack_size);
            }
        }
        stack_size = generate_stack_posistions(scope, child, stack_size);
    }
    ((function*)scope->data)->frame_size = stack_size;
    return stack_size;
    
}


// This will be useful if we do constant folding
int evaluate_node(AST_node* n)
{
    if (n->children->count == 0) {
        return strtol(n->token->data, NULL, 10);
    } else {
        if (n->children->count != 2) {
            fprintf(stderr, "%s:%d: error: Evaluating expression with not two nodes\n", __FILE__, __LINE__);
        }
        int left = evaluate_node(((AST_node**)n->children->data)[0]);
        int right = evaluate_node(((AST_node**)n->children->data)[1]);
        
        TOKEN_TYPE ty = ((operator*)n->data)->type;
        switch (ty) {
        case PLUS: {
            return left + right;
            break;
        }
        case MINUS: {
            return left - right;
            break;
        }
        case LOGICAL_AND: {
            return left && right;
            break;
        }
        case LOGICAL_OR: {
            return left || right;
            break;
        }
        default: {
            fprintf(stderr, "%s:%d: todo: Evaluating %s %i is not handled yet\n", __FILE__, __LINE__, n->token->data, n->token->pos_in_file);
            break;
        }
        }
            
    }
    fprintf(stderr, "%s:%d: error: Evaluating %s %i was not handled\n", __FILE__, __LINE__, n->token->data, n->token->pos_in_file);
    return -1;
}

