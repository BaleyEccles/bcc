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

AST_node* get_varible_node_from_name(AST_node* node, char* name) {
    AST_node* output = get_varible_node_from_name_recursive(node, name);
    if (output == NULL) {
        fprintf(stderr, "%s:%d: error: Unable to find varible from name %s\n", __FILE__, __LINE__, name);
    }
    return output;
}

AST_node* get_varible_node_from_name_recursive(AST_node* node, char* name) {
    if (node->node_type == VARIBLE) {
        if (strcmp(((varible*)node->data)->name, name) == 0) {
            return node;
        }
    }

    for (int i = 0; i < node->children->count; i++) {
        if (get_varible_node_from_name_recursive(((AST_node**)node->children->data)[i], name) != NULL) {
            return get_varible_node_from_name_recursive(((AST_node**)node->children->data)[i], name);
        }
    }
    return NULL;
}

bool is_varible_defined(AST_node* node, char* str) {
    if (node->node_type == VARIBLE) {
        if (strcmp(((varible*)node->data)->name, str) == 0) {
            return true;
        }
    }
    for (int i = 0; i < node->children->count; i++) {
        if (is_varible_defined(((AST_node**)node->children->data)[i], str)) {
            return true;
        }
    }
    return false;
}


int get_opening_paren_location(dynamic_array* tokens, int starting_token_location)
{
    if (((token**)tokens->data)[starting_token_location]->type != PAREN_CLOSE && ((token**)tokens->data)[starting_token_location]->type != PAREN_CURLY_CLOSE && ((token**)tokens->data)[starting_token_location]->type != PAREN_SQUARE_CLOSE) {
        fprintf(stderr, "%s:%d: error: input to get_opening_paren_location was not a parenthese, it was %s with type %i\n", __FILE__, __LINE__, ((token**)tokens->data)[starting_token_location]->data, ((token**)tokens->data)[starting_token_location]->type);
    }
    int i = starting_token_location;
    int paren_count = 1;
    TOKEN_TYPE token_type = ((token**)tokens->data)[starting_token_location]->type;
    while (paren_count != 0) {
        i--;
        if (i < 0) {
            fprintf(stderr, "%s:%d: error: Unable to find closing paren for %s, at index %i and location %i\n", __FILE__, __LINE__, ((token**)tokens->data)[starting_token_location]->data, starting_token_location, ((token**)tokens->data)[starting_token_location]->pos_in_file);
        }
        switch (token_type) {
        case PAREN_CLOSE: {
            if (((token**)tokens->data)[i]->type == PAREN_CLOSE) {
                paren_count++;
            } else if (((token**)tokens->data)[i]->type == PAREN_OPEN) {
                paren_count--;
            }
            break;
        }
        case PAREN_CURLY_CLOSE: {
            if (((token**)tokens->data)[i]->type == PAREN_CURLY_CLOSE) {
                paren_count++;
            } else if (((token**)tokens->data)[i]->type == PAREN_CURLY_OPEN) {
                paren_count--;
            }
            break;
        }
        case PAREN_SQUARE_CLOSE: {
            if (((token**)tokens->data)[ + i]->type == PAREN_SQUARE_CLOSE) {
                paren_count++;
            } else if (((token**)tokens->data)[ + i]->type == PAREN_SQUARE_OPEN) {
                paren_count--;
            }
            break;
        }
        default: {}
        }

    }
    return i;
}

int get_closing_paren_location(dynamic_array* tokens, int starting_token_location)
{
    if (((token**)tokens->data)[starting_token_location]->type != PAREN_OPEN && ((token**)tokens->data)[starting_token_location]->type != PAREN_CURLY_OPEN && ((token**)tokens->data)[starting_token_location]->type != PAREN_SQUARE_OPEN) {
        fprintf(stderr, "%s:%d: error: input to get_closing_paren_location was not a parenthese, it was %s with type %i\n", __FILE__, __LINE__, ((token**)tokens->data)[starting_token_location]->data, ((token**)tokens->data)[starting_token_location]->type);
    }
    
    int i = starting_token_location;
    int paren_count = 1;
    TOKEN_TYPE token_type = ((token**)tokens->data)[starting_token_location]->type;
    while (paren_count != 0) {
        i++;
        if (i >= tokens->count) {
            fprintf(stderr, "%s:%d: error: Unable to find closing paren for %s, at index %i and location %i\n", __FILE__, __LINE__, ((token**)tokens->data)[starting_token_location]->data, starting_token_location, ((token**)tokens->data)[starting_token_location]->pos_in_file);
        }
        switch (token_type) {
        case PAREN_OPEN: {
            if (((token**)tokens->data)[i]->type == PAREN_CLOSE) {
                paren_count--;
            } else if (((token**)tokens->data)[i]->type == PAREN_OPEN) {
                paren_count++;
            }
            break;
        }
        case PAREN_CURLY_OPEN: {
            if (((token**)tokens->data)[i]->type == PAREN_CURLY_CLOSE) {
                paren_count--;
            } else if (((token**)tokens->data)[i]->type == PAREN_CURLY_OPEN) {
                paren_count++;
            }
            break;
        }
        case PAREN_SQUARE_OPEN: {
            if (((token**)tokens->data)[ + i]->type == PAREN_SQUARE_CLOSE) {
                paren_count--;
            } else if (((token**)tokens->data)[ + i]->type == PAREN_SQUARE_OPEN) {
                paren_count++;
            }
            break;
        }
        default: {}
        }

    }
    return i;
}

AST_node* create_main_function_node(dynamic_array* tokens)
{
    AST_node* main_node = malloc(sizeof(AST_node));

    for (int i = 1; i < tokens->count - 1; i++) {
        if (((token**)tokens->data)[i]->type == OTHER &&
            strcmp(((token**)tokens->data)[i]->data, "main") == 0 &&
            ((token**)tokens->data)[i-1]->type == OTHER &&
            ((token**)tokens->data)[i+1]->type == PAREN_OPEN
            ) {
            
            function* main_function = malloc(sizeof(function));
            //main_function->return_type = get_type_from_str(((token**)tokens->data)[i - 1]->data);
            main_function->name = ((token**)tokens->data)[i]->data;

            // TODO: function that parses function inputs
            // function_input parse_function_inputs(tokens, i, tokens_until_function_start);

            init_AST_node(main_node);
            
            main_node->token = ((token**)tokens->data)[i];
            main_node->node_type = FUNCTION;
            main_node->data = (void*)(main_function);
        }
    }
    if (main_node == NULL) {
        fprintf(stderr, "%s:%d: error: Unable to find main function\n", __FILE__, __LINE__);
    }

    return main_node;
}



// TODO: This function will messup when we add parentheses
// Also, this is a bad name for the function
AST_node* create_single_rvalue_node(AST_node* scope, dynamic_array* tokens, int start_location, int end_location)
{
    if (end_location - start_location > 1) {
        fprintf(stderr, "%s:%d: error: More than one value in 'create_single_rvalue_node', this has not been delt with yet\nThe tokens are:\n", __FILE__, __LINE__);
        for (int i = start_location; i < end_location; i++) {
            fprintf(stderr, "    %s\n", ((token**)tokens->data)[i]->data);
        }
    }

    token* t = ((token**)tokens->data)[start_location];
    AST_node* node = malloc(sizeof(AST_node));
    init_AST_node(node);

    // TODO: Better type handling and stuff
    if (token_is_number(t)) {
        node->node_type = CONSTANT;
        node->token = t;
        constant* c = malloc(sizeof(constant));
        
        // TODO: Type stuff
        //c->type =
        c->value = t->data;
        node->data = (void*)c;
    } else {
        if (!is_varible_defined(scope, t->data)) {
            fprintf(stderr, "%s:%d: error: Varible '%s' on posistion %i is not defined\n", __FILE__, __LINE__, t->data, t->pos_in_file);
        }
        node = get_varible_node_from_name(scope, t->data);
        // TODO: For now we assume that it is a varible, if not a constant
        node->node_type = VARIBLE;
        node->token = t;
        varible* v = malloc(sizeof(varible));
        
        // TODO: Type stuff
        //v->type =
        v->name = t->data;
        node->data = (void*)v;
        
    }
          
    return node;
}

AST_node* create_expression_node(AST_node* scope, dynamic_array* tokens, int start_location, int end_location)
{

    for (int i = sizeof(operator_mapping)/sizeof(operator_mapping[0]); i >= 0; i--) {
        for (int j = start_location; j < end_location + 1; j++) {
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

                int left_start = start_location;
                int left_end = j - 1;
                AST_node* left_node = create_expression_node(scope, tokens, left_start, left_end);
                da_append(node->children, left_node, AST_node*);
                if (o->type == POST_INCREMENT || o->type == POST_DECREMENT) {
                } else {
                    int right_start = j + 1;
                    int right_end = end_location;
                    AST_node* right_node = create_expression_node(scope, tokens, right_start, right_end);
                    da_append(node->children, right_node, AST_node*);
                }
                
                return node;
            }

        }
        
    }
    // There are no operators in the currnet token list
    // We have reached a constant number, string, char, etc. Now we must make and return it.
    //printf("start loc: %i, end loc: %i\n", start_location, end_location);
    return create_single_rvalue_node(scope, tokens, start_location, end_location);
}

int generate_stack_posistions(AST_node* scope, AST_node* node , int stack_size)
{
    for (int i = 0; i < node->children->count; i++) {
        AST_node* child = ((AST_node**)node->children->data)[i];
        if (child->node_type == VARIBLE) {
            if (((varible*)child->data)->stack_pos == 0) {
                // TOOD: Deal with different types / different sizes
                stack_size += 4;
                ((varible*)child->data)->stack_pos = stack_size;
            }
        }
        stack_size = generate_stack_posistions(scope, child, stack_size);
    }
    
    return stack_size;
    
}

void create_varible_init_node(AST_node* scope, AST_node* node, dynamic_array* tokens, int loc)
{
    AST_node* assign_node = malloc(sizeof(AST_node));
    init_AST_node(assign_node);
            
    assign_node->token = ((token**)tokens->data)[loc + 2];
    assign_node->node_type = OPERATOR;
            
    operator* o = malloc(sizeof(operator));
    o->type = ((token**)tokens->data)[loc + 2]->type;
    o->name = ((token**)tokens->data)[loc + 2]->data;
    assign_node->data = (void*)o;

            
    AST_node* varible_node = malloc(sizeof(AST_node));
    init_AST_node(varible_node);
            
    varible_node->token = ((token**)tokens->data)[loc + 1];
    varible_node->node_type = VARIBLE;

    varible* v = malloc(sizeof(varible));
    //v->type = get_type_from_str(types, ((token**)tokens->data)[i]->data);
    v->name = ((token**)tokens->data)[loc + 1]->data;
    varible_node->data = (void*)(v);

    int rhs_end_location = find_semi_colon(tokens, loc) - 1;
    AST_node* rvalue_node = create_expression_node(node, tokens, loc + 3, rhs_end_location);

    da_append(assign_node->children, varible_node, AST_node*);
    da_append(assign_node->children, rvalue_node, AST_node*);
                                                                         
    da_append(node->children, assign_node, AST_node*);
}

void create_modify_varible_node(AST_node* scope, AST_node* node, dynamic_array* tokens, int loc)
{
    
    if (!is_varible_defined(node, ((token**)tokens->data)[loc + 1]->data)) {
        fprintf(stderr, "%s:%d: error: Varible '%s' on posistion %i is not defined\n", __FILE__, __LINE__, ((token**)tokens->data)[loc + 1]->data, ((token**)tokens->data)[loc + 1]->pos_in_file);
    }
    AST_node* varible_node = get_varible_node_from_name(node, ((token**)tokens->data)[loc + 1]->data);
    AST_node* assign_node = malloc(sizeof(AST_node));
    init_AST_node(assign_node);
            
    assign_node->token = ((token**)tokens->data)[loc + 2];
    assign_node->node_type = OPERATOR;
            
    operator* operator = malloc(sizeof(operator));
    operator->type = ((token**)tokens->data)[loc + 2]->type;
    operator->name = ((token**)tokens->data)[loc + 2]->data;
    assign_node->data = (void*)(operator);
        
    int rhs_end_location = find_semi_colon(tokens, loc) - 1;
    AST_node* rvalue_node = create_expression_node(node, tokens, loc + 3, rhs_end_location);
            
    da_append(assign_node->children, varible_node, AST_node*);
    da_append(assign_node->children, rvalue_node, AST_node*);
    da_append(node->children, assign_node, AST_node*);
}

void create_return_node(AST_node* scope, AST_node* node, dynamic_array* tokens, int loc)
{
    AST_node* key_word_node = malloc(sizeof(AST_node));
    init_AST_node(key_word_node);

    key_word_node->node_type = KEY_WORD;
    key_word_node->token = ((token**)tokens->data)[loc];
                
    key_word* kw = malloc(sizeof(key_word));
    kw->name = ((token**)tokens->data)[loc]->data;
    kw->key_word_type = RETURN;
    kw->data = NULL;
    key_word_node->data = kw;

    int return_end_location = find_semi_colon(tokens, loc) - 1;
    AST_node* rvalue_node = create_expression_node(node, tokens, loc + 1, return_end_location);

    da_append(key_word_node->children, rvalue_node, AST_node*);
    da_append(node->children, key_word_node, AST_node*);
}

int create_if_node(AST_node* scope, AST_node* node, dynamic_array* tokens, int loc)
{
    AST_node* key_word_node = malloc(sizeof(AST_node));
    init_AST_node(key_word_node);

    key_word_node->node_type = KEY_WORD;
    key_word_node->token = ((token**)tokens->data)[loc];
    key_word* kw = malloc(sizeof(key_word));
    kw->key_word_type = IF;
    kw->name = ((token**)tokens->data)[loc]->data;
    kw->data = NULL;
    key_word_node->data = kw;

    if (((token**)tokens->data)[loc + 1]->type != PAREN_OPEN) {
        fprintf(stderr, "%s:%d: error: Expected '(' after if statement at location %i\n", __FILE__, __LINE__, ((token**)tokens->data)[loc]->pos_in_file);
    }

    int if_condition_start_location = loc + 2;
    int if_condition_end_location = get_closing_paren_location(tokens, if_condition_start_location - 1) - 1;
    AST_node* if_condition_node = create_expression_node(node, tokens, if_condition_start_location, if_condition_end_location);

    da_append(key_word_node->children, if_condition_node, AST_node*);

    int if_body_start_location = if_condition_end_location + 3;
    if (((token**)tokens->data)[if_body_start_location - 1]->type != PAREN_CURLY_OPEN) {
        fprintf(stderr, "%s:%d: error: Expected '{' after if condition at location %i\n", __FILE__, __LINE__, ((token**)tokens->data)[if_body_start_location - 1]->pos_in_file);
    }
    int if_body_end_location = get_closing_paren_location(tokens, if_body_start_location - 1) - 1;

    create_body_AST_node(scope, key_word_node, tokens, if_body_start_location, if_body_end_location);

    if (((token**)tokens->data)[loc - 1]->type == ELSE) {
        AST_node* else_node = get_node_from_pos(node, ((token**)tokens->data)[loc - 1]->pos_in_file);
        da_append(else_node->children, key_word_node, AST_node*);
    } else {
        da_append(node->children, key_word_node, AST_node*);
    }
                
    return (if_body_end_location + 1);
}


int create_else_node(AST_node* scope, AST_node* node, dynamic_array* tokens, int loc)
{
    if (((token**)tokens->data)[loc - 1]->type != PAREN_CURLY_CLOSE) {
        fprintf(stderr, "%s:%d: error: Expected '}' before else condition at location %i\n", __FILE__, __LINE__, ((token**)tokens->data)[loc - 1]->pos_in_file);
    }
    int if_location = get_opening_paren_location(tokens, get_opening_paren_location(tokens, loc - 1) - 1) - 1;
    AST_node* if_node = get_node_from_pos(node, ((token**)tokens->data)[if_location]->pos_in_file);

    AST_node* else_node = malloc(sizeof(AST_node));
    init_AST_node(else_node);
                
    else_node->node_type = KEY_WORD;
    else_node->token = ((token**)tokens->data)[loc];
    key_word* kw = malloc(sizeof(key_word));
    kw->key_word_type = ELSE;
    kw->name = ((token**)tokens->data)[loc]->data;
    kw->data = NULL;
    else_node->data = kw;

    da_append(if_node->children, else_node, AST_node*);
                
    if (((token**)tokens->data)[loc + 1]->type == PAREN_CURLY_OPEN) {
                    
        int else_body_start_location = loc + 2;
        if (((token**)tokens->data)[else_body_start_location - 1]->type != PAREN_CURLY_OPEN) {
            fprintf(stderr, "%s:%d: error: Expected '{' after else condition at location %i\n", __FILE__, __LINE__, ((token**)tokens->data)[else_body_start_location - 1]->pos_in_file);
        }
        int else_body_end_location = get_closing_paren_location(tokens, else_body_start_location - 1) - 1;
        create_body_AST_node(scope, else_node, tokens, else_body_start_location, else_body_end_location);
        return else_body_end_location + 1;
    }
    return loc;
}

int create_for_node(AST_node* scope, AST_node* node, dynamic_array* tokens, int loc)
{
    if (((token**)tokens->data)[loc + 1]->type != PAREN_OPEN) {
        fprintf(stderr, "%s:%d: error: Expected '(' after for loop start at location %i\n", __FILE__, __LINE__, ((token**)tokens->data)[loc - 1]->pos_in_file);
    }
    AST_node* for_node = malloc(sizeof(AST_node));
    init_AST_node(for_node);
    
    for_node->node_type = KEY_WORD;
    for_node->token = ((token**)tokens->data)[loc];
    key_word* kw = malloc(sizeof(key_word));
    kw->key_word_type = FOR;
    kw->name = ((token**)tokens->data)[loc]->data;
    kw->data = NULL;
    for_node->data = kw;
    da_append(node->children, for_node, AST_node*);
    
    int for_loop_end = get_closing_paren_location(tokens, loc + 1);
    
    int for_loop_prelude_start = loc + 2;
    int for_loop_prelude_end = find_semi_colon(tokens, for_loop_prelude_start);
    if (for_loop_end < for_loop_prelude_end) {
        fprintf(stderr, "%s:%d: error: Failed to parse the for loop at location %i\n", __FILE__, __LINE__, ((token**)tokens->data)[loc - 1]->pos_in_file);
    }
    
    create_body_AST_node(scope, for_node, tokens, for_loop_prelude_start, for_loop_prelude_end);
    //da_append(for_node->children, prelude_node, AST_node*);
    
    int for_loop_condition_start = for_loop_prelude_end + 1;
    int for_loop_condition_end = find_semi_colon(tokens, for_loop_condition_start);
    if (for_loop_end < for_loop_condition_end) {
        fprintf(stderr, "%s:%d: error: Failed to parse the for loop at location %i\n", __FILE__, __LINE__, ((token**)tokens->data)[loc - 1]->pos_in_file);
    }
    AST_node* condition_node = create_expression_node(scope, tokens, for_loop_condition_start, for_loop_condition_end - 1);
    da_append(for_node->children, condition_node, AST_node*);
    
    int for_loop_epilogue_start = for_loop_condition_end + 1;
    int for_loop_epilogue_end = for_loop_end - 1;
    if (for_loop_end < for_loop_epilogue_end) {
        fprintf(stderr, "%s:%d: error: Failed to parse the for loop at location %i\n", __FILE__, __LINE__, ((token**)tokens->data)[loc - 1]->pos_in_file);
        printf("l1 %i, l2 %i\n", for_loop_end , for_loop_epilogue_end);
    }
    AST_node* epilogue_node = create_expression_node(scope, tokens, for_loop_epilogue_start, for_loop_epilogue_end);
    da_append(for_node->children, epilogue_node, AST_node*);

    int for_loop_body_start = for_loop_epilogue_end + 2;
    int for_loop_body_end = get_closing_paren_location(tokens, for_loop_body_start);
    create_body_AST_node(scope, for_node, tokens, for_loop_body_start, for_loop_body_end);
    
    return (for_loop_body_end);
}


int create_key_word_node(AST_node* scope, AST_node* node, dynamic_array* tokens, int loc)
{
    // TODO: add more key words
    TOKEN_TYPE key_word_type = get_token_type_key_word(((token**)tokens->data)[loc]);
    switch (key_word_type) {
    case RETURN: {
        create_return_node(scope, node, tokens, loc);
        break;
    }
    case IF: {
        loc = create_if_node(scope, node, tokens, loc);
        break;
    }
    case ELSE: {
        loc = create_else_node(scope, node, tokens, loc);
        break;
    }
    case FOR: {
        loc = create_for_node(scope, node, tokens, loc);
        break;
    }
    default: {
        fprintf(stderr, "%s:%d: TODO: Key word '%s' was not handled\n", __FILE__, __LINE__, ((token**)tokens->data)[loc]->data);
        break;
    }
    }
    return loc;
}



AST_node* create_body_AST_node(AST_node* scope, AST_node* node, dynamic_array* tokens, int start, int end)
{
    for (int i = start; i < end; i++) {
        if (((token**)tokens->data)[i]->type == OTHER &&
            ((token**)tokens->data)[i + 1]->type == OTHER &&
            ((token**)tokens->data)[i + 2]->type == EQUALS) {
            // Initalize varible
            create_varible_init_node(scope, node, tokens, i);
            i = find_semi_colon(tokens, i);
        } else if (((token**)tokens->data)[i + 0]->type != OTHER &&
                   ((token**)tokens->data)[i + 1]->type == OTHER &&
                   ((token**)tokens->data)[i + 2]->type == EQUALS) {
            // Modify varible
            create_modify_varible_node(scope, node, tokens, i);
            i = find_semi_colon(tokens, i) - 1;

        } else if (token_is_key_word(((token**)tokens->data)[i])) {
            i = create_key_word_node(scope, node, tokens, i);
        } else if (((token**)tokens->data)[i + 0]->type != OTHER &&
                   ((token**)tokens->data)[i + 1]->type == OTHER &&
                   ((token**)tokens->data)[i + 2]->type != EQUALS) {
            int end = find_semi_colon(tokens, i) - 1;
            AST_node* n = create_expression_node(scope, tokens, i + 1, end);
            da_append(node->children, n, AST_node*);
            i = end;
        }

    }
    
    return node;
}


