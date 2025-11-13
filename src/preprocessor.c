#include "preprocessor.h"

typedef struct {
    PREPROCESSORS t;
    char* str;
} str_enum_map;

static const str_enum_map preprocessors[] = {
    {PRE_PROCESS_DEFINE, "define"}, {PRE_PROCESS_INCLUDE, "include"}, {PRE_PROCESS_IF, "if"}, {PRE_PROCESS_ELSE, "else"}, {PRE_PROCESS_ELIF, "elif"}, {PRE_PROCESS_IFDEF, "ifdef"}, {PRE_PROCESS_IFNDEF, "ifndef"}, {PRE_PROCESS_ENDIF, "endif"}, {PRE_PROCESS_UNDEF, "undef"}, {PRE_PROCESS_ERROR, "error"}
};


PREPROCESSORS get_preproccessor(dynamic_array* tokens, int location)
{
    token* t1 = ((token**)tokens->data)[location];
    token* t2 = NULL;
    for (int i = location + 1; i < tokens->count; i++) {
        t2 = ((token**)tokens->data)[i];
        if (strcmp(t2->data, " ") != 0) {
            break;
        }
    }

    if (t2 != NULL) {
        for (size_t i = 0; i < sizeof(preprocessors)/sizeof(preprocessors[0]); i++) {
            char* type = preprocessors[i].str;
            if (strcmp(t2->data, type) == 0 && strcmp(t1->data, "#") == 0) {
                return i;
            }
        }
    }
    return -1;
}

int get_end_of_define(dynamic_array* tokens, int start)
{
    token* t;
    token* t_next;
    for (int i = start; i < tokens->count; i++) {
        t_next = ((token**)tokens->data)[i + 1];
        t = ((token**)tokens->data)[i];
        if (strcmp(t_next->data, "\n") == 0 && strcmp(t->data, "\\") != 0) {
            return i + 1;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find end of preprocessor define\n", __FILE__, __LINE__);
    return -1;
}

int get_end_of_line(dynamic_array* tokens, int start)
{
    token* t;
    for (int i = start; i < tokens->count; i++) {
        t = ((token**)tokens->data)[i];
        if (strcmp(t->data, "\n") == 0) {
            return i;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find end of preprocessor define\n", __FILE__, __LINE__);
    return -1;
}


void replace_tokens_with_array(dynamic_array* tokens_dst, token* t, int start_range, int end_range, dynamic_array* tokens_src)
{
    int len = tokens_src->count;
    for (int i = start_range; i < end_range; i++) {
        token* current_token = ((token**)tokens_dst->data)[i];
        if (strcmp(current_token->data, t->data) == 0) {
            da_shift(tokens_dst, token*, i, len - 1);
            for (int j = 0; j < len; j++) {
                token* new_token = malloc(sizeof(token));
                memcpy(new_token, ((token**)tokens_src->data)[j], sizeof(token));
                ((token**)tokens_dst->data)[j + i] = new_token;
            }
        }
    }
}
    
void replace_tokens(dynamic_array* tokens, token* t, int start_range, int end_range, int start, int end)
{
    int len = end - start;

    for (int i = start_range; i < end_range; i++) {
        token* current_token = ((token**)tokens->data)[i];
        if (strcmp(current_token->data, t->data) == 0) {
            da_shift(tokens, token*, i, len - 1);
            for (int j = 0; j < len; j++) {
                token* new_token = malloc(sizeof(token));
                memcpy(new_token, ((token**)tokens->data)[j + start], sizeof(token));
                
                ((token**)tokens->data)[j + i] = new_token;
                if (j + i > tokens->count || j + start > tokens->count) {
                    fprintf(stderr, "%s:%d: error: Messed up ranges in replace_tokens\n", __FILE__, __LINE__);
                }
        
            
            }
        }
    }
    
}

void remove_tokens(dynamic_array* tokens, int start, int end)
{
    token* t = malloc(sizeof(token));
    ALLOC_STR(t, "");
    
    for (int i = start; i < end; i++) {
        ((token**)tokens->data)[i] = t;
    }
}

void create_define(dynamic_array* tokens, define* d, int start, int end)
{
    d->inputs = NULL;
    
    dynamic_array* define_output = malloc(sizeof(dynamic_array));
    da_init(define_output, token*);
    for (int i = start + 1; i < end; i++) {
        da_append(define_output, ((token**)tokens->data)[i], token*);
    }
    d->output = define_output;
}

void create_macro(dynamic_array* tokens, define* d, int macro_input_start, int macro_input_end, int end)
{
    dynamic_array* macro_inputs = malloc(sizeof(dynamic_array));
    da_init(macro_inputs, token*);
    for (int i = macro_input_start; i < macro_input_end; i++) {
        da_append(macro_inputs, ((token**)tokens->data)[i], token*);
    }
    clean_tokens(macro_inputs);
    for (int i = 0; i < macro_inputs->count; i++) {
        if (strcmp(((token**)macro_inputs->data)[i]->data, ",") == 0) {
            for (int j = i; j < macro_inputs->count; j++) {
                ((token**)macro_inputs->data)[j] = ((token**)macro_inputs->data)[j + 1];
            }
            i--;
            (macro_inputs->count)--;
        }
    }
    // macro_inputs:
    // [a] [b] [c] ...
    d->inputs = macro_inputs;

    dynamic_array* macro_output = malloc(sizeof(dynamic_array));
    da_init(macro_output, token*);    
    for (int i = macro_input_end + 2; i < end; i++) {
        da_append(macro_output, ((token**)tokens->data)[i], token*);
    }
    d->output = macro_output;
}

char* get_path(dynamic_array* paths, char* name) {
    for (int i = 0; i < paths->count; i++) {
        char* str = ((char**)paths->data)[i];
        char* path = malloc(sizeof(char) * (strlen(name) + strlen(str) + 2));
        if (path == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return NULL;
        }
    
        strcpy(path, str);
        path[strlen(str)] = '/';
        strcpy(path + strlen(str) + 1, name);
    
        path[strlen(name) + strlen(str) + 1] = '\0';
    
        // Check if the path exists
        if (access(path, F_OK) == 0) {
            return path;
        }
    
        free(path);
    }

    fprintf(stderr, "%s:%d: error: Unable to find file %s\n", __FILE__, __LINE__, name);
    return NULL; // Return NULL instead of an empty string
}

int find_else(dynamic_array* tokens, int start, int end)
{

    for (int i = start; i < end; i++) {
        token* t = ((token**)tokens->data)[i];
        if (strcmp(t->data, "#") == 0) {
            i++;
            t = ((token**)tokens->data)[i];
            while (strcmp(t->data, " ") == 0) {
                i++;
                t = ((token**)tokens->data)[i];
            }

            if (t->data[0] == 'i' && t->data[1] == 'f') {
                i = find_endif(tokens, i) + 1;
            }
            if (t->data[0] == 'e' && t->data[1] == 'l') {
                return i;
            }
        }
    }
    return end;
}


int find_endif(dynamic_array* tokens, int start)
{
    int loc = 0;
    int if_count = 1;
    for (int i = start; i < tokens->count; i++) {
        token* t = ((token**)tokens->data)[i];
        if (strcmp(t->data, "#") == 0) {
            i++;
            t = ((token**)tokens->data)[i];
            while (strcmp(t->data, " ") == 0) {
                i++;
                t = ((token**)tokens->data)[i];
            }

            if (t->data[0] == 'i' && t->data[1] == 'f') {
                if_count++;
            }
            if (strcmp(t->data, "endif") == 0) {
                if_count--;
            }
        }
        if (if_count == 0) {
            loc = i;
            return loc;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find endif for %s %i\n", __FILE__, __LINE__, ((token**)tokens->data)[start]->data, ((token**)tokens->data)[start]->pos_in_file);
    *(int*)0 = 0;
    return -1;
}

int find_hash(dynamic_array* tokens, int endif_token_loc)
{
    for (int i = endif_token_loc; i >= 0; i--) {
        token* t = ((token**)tokens->data)[i];
        if (strcmp(t->data, "#") == 0) {
            return i;
            break;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find # for %s %i\n", __FILE__, __LINE__, ((token**)tokens->data)[endif_token_loc]->data, ((token**)tokens->data)[endif_token_loc]->pos_in_file);
    return -1;
}

void do_block(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int val, int start, int end, int token_loc, int endif_token_loc)
{
    if (val == 0) {
        int next_cond = find_else(tokens, token_loc, endif_token_loc);
        int next_cond_hash = find_hash(tokens, next_cond);
        remove_tokens(tokens, start, next_cond_hash - 1);
        
        int key = get_preproccessor(tokens, next_cond_hash);
        do_preprocessor(tokens, defines, include_paths, next_cond_hash, key);
    } else {
        int endif_hash = find_hash(tokens, endif_token_loc);
        int next_cond = find_else(tokens, token_loc, endif_token_loc);
        int next_cond_hash = find_hash(tokens, next_cond);
        
        remove_tokens(tokens, start, end);
        remove_tokens(tokens, next_cond_hash, endif_hash);
        remove_tokens(tokens, endif_hash, endif_token_loc + 1);
    }
}

bool is_defined(dynamic_array* defines, token* define_token)
{
    bool defined = false;
    for (int i = 0; i < defines->count; i++) {
        define* d = ((define**)defines->data)[i];
        if (d != NULL) {
            if (strcmp(d->name->data, define_token->data) == 0) {
                defined = true;
                break;
            }
        }
    }
    return defined;
}

int find_token_loc(dynamic_array* tokens, int start, int end, char* name)
{
    for (int i = start; i < end; i++) {
        token* t = ((token**)tokens->data)[i];
        if (strcmp(t->data, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find %s\n", __FILE__, __LINE__, name);
    return -1;
}

token* find_define_token(dynamic_array* tokens, int start, int end)
{
    if (strcmp(((token**)tokens->data)[start]->data, "") == 0) {
         for (int i = start; i < end; i++) {
            token* t1 = ((token**)tokens->data)[i - 1];
            token* t2 = ((token**)tokens->data)[i + 0];
            
            if (strcmp(t1->data, "") == 0 && strcmp(t2->data, "") != 0) {
                return t2;
                break;
            }
        }
    } else {
        for (int i = start; i < end; i++) {
            token* t1 = ((token**)tokens->data)[i - 1];
            token* t2 = ((token**)tokens->data)[i + 0];
            if (strcmp(t1->data, " ") == 0 && strcmp(t2->data, " ") != 0) {
                return t2;
                break;
            }
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find define token\n", __FILE__, __LINE__);

    return NULL;
}

void do_preprocessor_ifndef(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int start, int end)
{
    
    int ifndef_token_loc = find_token_loc(tokens, start, end, "ifndef");
    
    token* define_token = find_define_token(tokens, ifndef_token_loc + 1, end);
    
    int endif_token_loc = find_endif(tokens, ifndef_token_loc + 1);
    
    bool defined = is_defined(defines, define_token);
    
    do_block(tokens, defines, include_paths, !defined, start, end, ifndef_token_loc, endif_token_loc);
}
void do_preprocessor_ifdef(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int start, int end)
{

    int ifdef_token_loc = find_token_loc(tokens, start, end, "ifdef");

    token* define_token = find_define_token(tokens, ifdef_token_loc + 1, end);
    
    int endif_token_loc = find_endif(tokens, ifdef_token_loc + 1);
    
    bool defined = is_defined(defines, define_token);
    
    do_block(tokens, defines, include_paths, defined, start, end, ifdef_token_loc, endif_token_loc);
}

int evaluate_expression(dynamic_array* tokens, dynamic_array* defines, int start, int end)
{
    dynamic_array* ts = malloc(sizeof(dynamic_array));
    da_init(ts, token*);
    for (int i = start; i < end + 1; i++) {
        da_append(ts, ((token**)tokens->data)[i], token*);
    }

    clean_tokens(ts);
       
    for (int i = 0; i < ts->count; i++) {
        token* t = ((token**)ts->data)[i];
        if (strcmp(t->data, "defined") == 0) {
            int loc = i + 1;
            //int end_defined = loc;
            token* t2 = ((token**)ts->data)[loc];
            if (strcmp(t2->data, "(") == 0) {
                loc += 1;
                //end_defined = loc + 1;
                t2 = ((token**)ts->data)[loc];
            }
            
            bool defined = is_defined(defines, t2);
            
            dynamic_array* da = malloc(sizeof(dynamic_array));
            da_init(da, token*);
            if (defined) {
                token* token_defined = malloc(sizeof(token));
                ALLOC_STR(token_defined, "1");
                da_append(da, token_defined, token*);

            } else {
                token* token_defined = malloc(sizeof(token));
                ALLOC_STR(token_defined, "0");
                da_append(da, token_defined, token*);
            }
            
            replace_tokens_with_array(ts, t2, loc, loc + 1, da);
            da_destroy(da, token*);
            remove_tokens(ts, i, i + 1);
            
        }
    }


    for (int j = 0; j < defines->count; j++) {
        define* d = ((define**)defines->data)[j];
        if (d != NULL) {
            
            for (int i = 0; i < d->output->count; i++) {
            }
            

        }
    }
    for (int i = 0; i < ts->count; i++) {
        token* t = ((token**)ts->data)[i];
        for (int j = 0; j < defines->count; j++) {
            define* d = ((define**)defines->data)[j];
            if (d != NULL) {
                if (strcmp(d->name->data, t->data) == 0) {
                    replace_token_with_define(ts, d, i);
                }
            }
        }
    }
    
    for (int i = 0; i < ts->count; i++) {
        token* t = ((token**)ts->data)[i];
        for (int j = 0; j < defines->count; j++) {
            define* d = ((define**)defines->data)[j];
            if (d != NULL) {
                if (strcmp(d->name->data, t->data) == 0) {
                    replace_token_with_define(ts, d, i);
                }
            }
        }
    }

    clean_tokens(ts);
    for (int i = 0; i < ts->count; i++) {
        get_token_type(NULL, ts, ((token**)ts->data)[i]); // TODO
    }
    for (int i = 0; i < ts->count; i++) {
    }
        

    context* ctx = malloc(sizeof(context));
    ctx->tokens = ts;
    ctx->functions = NULL;
    ctx->types = NULL;


    AST_node* n = create_expression_node(NULL, ctx, 0, ts->count - 1);
    free(ctx);
    da_destroy(ts, token*);
    
    generate_graphviz_from_AST_node(n, "a.gv");
    
    return evaluate_node(n);

}

void do_preprocessor_if(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int start, int end)
{
    int if_token_loc = find_token_loc(tokens, start, end, "if");
    
    int endif_token_loc = find_endif(tokens, if_token_loc + 1);
    

    int val = evaluate_expression(tokens, defines, if_token_loc + 1, end);
    do_block(tokens, defines, include_paths, val, start, end, if_token_loc, endif_token_loc);
}

void do_preprocessor_elif(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int start, int end)
{
    int elif_token_loc = find_token_loc(tokens, start, end, "elif");
    
    int endif_token_loc = find_endif(tokens, elif_token_loc + 1);
    
    int val = evaluate_expression(tokens, defines, elif_token_loc + 1, end);
    do_block(tokens, defines, include_paths, val, start, end, elif_token_loc, endif_token_loc);
    
}

void do_preprocessor_else(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int start, int end)
{
    int else_token_loc = find_token_loc(tokens, start, end, "else");
    
    int endif_token_loc = find_endif(tokens, else_token_loc + 1);
    do_block(tokens, defines, include_paths, true, start, end, else_token_loc, endif_token_loc);
}


void do_preprocessor_include(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int start, int end)
{
    char* include_name = NULL;
    token* include_token = NULL;
    int first_space = -1;
    for (int i = start; i < end; i++) {
        token* t = ((token**)tokens->data)[i];
        if (first_space == -1 && strcmp(t->data, " ") == 0) {
            first_space = i;
        }
        if ((t->data[0] == '<' && t->data[strlen(t->data) - 1] == '>') ||
            (t->data[0] == '"' && t->data[strlen(t->data) - 1] == '"')) {
            include_name = malloc((strlen(t->data) + 1)*sizeof(char));
            strcpy(include_name, t->data);
            include_token = t;
            break;
        } else if (t->data[0] == '<' || t->data[0] == '"') {
            int idx = i;
            int count = 0;
            while (t->data[strlen(t->data) - 1] != '"' && t->data[strlen(t->data) - 1] != '>') {
                count++;
                i++;
                t = ((token**)tokens->data)[i];
            }
            count++;
            i++;
            t = ((token**)tokens->data)[i];
            
            int size = 0;
            for (int j = 0; j < count; j++) {
                t = ((token**)tokens->data)[idx + j];
                size += strlen(t->data);
            }
            include_name = malloc(sizeof(char)*(size + 1));
            include_token = t;
            size = 0;
            for (int j = 0; j < count; j++) {
                t = ((token**)tokens->data)[idx + j];
                strcpy(include_name + size*sizeof(char), t->data);
                size += strlen(t->data);
            }
            break;
        }
    }
    if (include_token == NULL) {
        fprintf(stderr, "%s:%d: error: Unable to find include name. The tokens are:\n", __FILE__, __LINE__);
        for (int i = start; i < end; i++) {
            fprintf(stderr, "token %i: %s\n", i, ((token**)tokens->data)[i]->data);
        }
        
    }
    // :TODO: Handle external files
    //bool external_file = false;
    //if (include_name[0] == '<') {
    //    external_file = true;
    //}
    int len = strlen(include_name);
    for (int i = 0; i < len; i++) {
        if (include_name[i] == '<' || include_name[i] == '>' || include_name[i] == '"') {
            for (int j = i; j < len; j++) {
                include_name[j] = include_name[j + 1];
            }
            i--;
            len--;
        }
    }
    
    char* file_name = get_path(include_paths, include_name);
    free(include_name);
    
    //printf("include file %s\n", file_name);
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        fprintf(stderr, "%s:%d: error: fopen failed\n", __FILE__, __LINE__);
    }
    
    int ok = fseek(file, 0, SEEK_END);
    if (ok != 0) {
        fprintf(stderr, "%s:%d: error: fseek failed\n", __FILE__, __LINE__);
    }
    int file_size = ftell(file);
    rewind(file);

    dynamic_array file_tokens;
    da_init(&file_tokens, token*);

    int pos = 0;
    while (pos < file_size) {
        token* t = get_next_token(file, &file_tokens, &pos);
        da_append(&file_tokens, t, token*);
    }
    fclose(file);
    untabbify_tokens(&file_tokens);
    remove_comments(&file_tokens);
    for (int i = 0; i < file_tokens.count; i++) {
        get_token_type(NULL, &file_tokens, ((token**)file_tokens.data)[i]);
    }


    for (int i = 0; i < file_tokens.count; i++) {
    }

    remove_tokens(tokens, start, first_space);
    replace_tokens_with_array(tokens, include_token, start, end, &file_tokens);

    free(file_name);
}

void do_preprocessor_undefine(dynamic_array* tokens, dynamic_array* defines, int start, int end)
{
    int i = find_token_loc(tokens, start, end, "undef");
    
    token* name = NULL;
    i++;
    for (i = i; i < end; i++) {
        name = ((token**)tokens->data)[i];
        if (strcmp(name->data, " ") != 0) {
            break;
        }
    }
    
    for (int j = 0; j < defines->count; j++) {
        define* d = ((define**)defines->data)[j];
        if (d != NULL) {
            if (strcmp(d->name->data, name->data) == 0) {
                ((define**)defines->data)[j] = NULL; // TODO: Memory leak
            }
        }
    }
    remove_tokens(tokens, start, end);
}


void replace_token_with_define(dynamic_array* tokens, define* d, int loc)
{
    token* current_token = ((token**)tokens->data)[loc];
    if (d->inputs != NULL) {
        if (strcmp(((token**)tokens->data)[loc + 1]->data, "(") != 0) {
            fprintf(stderr, "%s:%d: error: Next token is not a ( for macro %s, %i\n", __FILE__, __LINE__, current_token->data, current_token->pos_in_file);
        }
        
        dynamic_array* inputs = malloc(sizeof(dynamic_array)); // TODO: Memory leak
        da_init(inputs, dynamic_array*);
        int input_loc = loc + 2;
        int macro_end = get_closing_paren_location(tokens, input_loc - 1);
        while (input_loc < macro_end) {
            dynamic_array* input = malloc(sizeof(dynamic_array));
            da_init(input, token*);
            int comma_loc = find_comma(tokens, input_loc, macro_end - 1);
            if (token_is_parentheses(((token**)tokens->data)[comma_loc + 1])) {
                comma_loc++;
            }
                                                                               
            for (int j = input_loc; j < comma_loc; j++) {
                da_append(input, ((token**)tokens->data)[j], token*);
            }
            da_append(inputs, input, dynamic_array*);
            input_loc = comma_loc + 1;
        }

        if (inputs->count != d->inputs->count) {
            fprintf(stderr, "%s:%d: error: Mismatched input counts for macro %s %i with %i inputs and %s %i with %i inputs\n", __FILE__, __LINE__, current_token->data, current_token->pos_in_file, d->inputs->count, current_token->data, current_token->pos_in_file, inputs->count);
        }

        // We now have a mapping from definition inputs to application inputs
        // a -> [1 + 2]
        // b -> [5]
        // ...
        remove_tokens(tokens, loc + 1, macro_end + 1); // Remove macro inputs
        replace_tokens_with_array(tokens, current_token, loc - 1, loc + 1, d->output); // Replace macro name with macro output
        
        // Replace each input with each stored input
        macro_end += d->output->count;
        for (int j = 0; j < d->inputs->count; j++) {
            replace_tokens_with_array(tokens, ((token**)d->inputs->data)[j], loc, macro_end, ((dynamic_array**)inputs->data)[j]);
            macro_end += ((dynamic_array**)inputs->data)[j]->count;
        }
        free(inputs);
    } else {
        for (int i = 0; i < d->output->count; i++) {
        }

        replace_tokens_with_array(tokens, current_token, loc, loc + 1, d->output); // Replace define name with define output
    }
}

void do_preprocessor_define(dynamic_array* tokens, dynamic_array* defines, int start, int end)
{
    // We need to know if there is a space between MACRO and (.
    // This will determine if it is a function macro, or a simple find and replace
    // #define MACRO(a, b) a + b
    // ^start                  ^end
    token* type = NULL;
    int i;
    for (i = start + 1; i < tokens->count; i++) {
        type = ((token**)tokens->data)[i];
        if (strcmp(type->data, " ") != 0 && strcmp(type->data, " ") != 0) {
            break;
        }
    }
    token* name = NULL;
    for (i = i + 1; i < tokens->count; i++) {
        name = ((token**)tokens->data)[i];
        if (strcmp(name->data, " ") != 0 && strcmp(name->data, "") != 0) {
            break;
        }
    }
    
    define* d = malloc(sizeof(define));
    d->name = name;
    
    bool is_macro = false;
    token* next = ((token**)tokens->data)[i + 1];
    if (token_is_parentheses(next)) {
        is_macro = true;
    }
    
    if (is_macro) {
        int macro_inputs_start = i + 1;
        int macro_inputs_end = get_closing_paren_location(tokens, i + 1);
        create_macro(tokens, d, macro_inputs_start + 1, macro_inputs_end, end);
        remove_tokens(tokens, start, end);
    } else {
        create_define(tokens, d, i + 1, end);
        remove_tokens(tokens, start, end);
    }

    if (strcmp(d->name->data, "") == 0 || strcmp(d->name->data, " ") == 0) {
        fprintf(stderr, "%s:%d: error: Adding empty define\n", __FILE__, __LINE__);        
    }
    da_append(defines, d, define*);
}

void do_preprocessor(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int start, int key)
{
    str_enum_map p = preprocessors[key];
    switch (p.t) {
    case PRE_PROCESS_DEFINE: {
        int end = get_end_of_define(tokens, start);
        do_preprocessor_define(tokens, defines, start, end);
        break;
    }
    case PRE_PROCESS_UNDEF: {
        int end = get_end_of_line(tokens, start);
        do_preprocessor_undefine(tokens, defines, start, end);
        break;
    }
    case PRE_PROCESS_IFDEF: {
        int end = get_end_of_line(tokens, start);
        do_preprocessor_ifdef(tokens, defines, include_paths, start, end);
        break;
    }
    case PRE_PROCESS_IFNDEF: {
        int end = get_end_of_line(tokens, start);
        do_preprocessor_ifndef(tokens, defines, include_paths, start, end);
        break;
    }
    case PRE_PROCESS_IF: {
        int end = get_end_of_define(tokens, start);
        do_preprocessor_if(tokens, defines, include_paths, start, end);
        break;
    }
    case PRE_PROCESS_ELSE: {
        int end = get_end_of_line(tokens, start);
        do_preprocessor_else(tokens, defines, include_paths, start, end);
        break;
    }
    case PRE_PROCESS_ELIF: {
        int end = get_end_of_define(tokens, start);
        do_preprocessor_elif(tokens, defines, include_paths, start, end);
        break;
    }
    case PRE_PROCESS_ENDIF: {
        int end = get_end_of_line(tokens, start);
        remove_tokens(tokens, start, end);
        break;
    }   
    case PRE_PROCESS_INCLUDE: {
        int end = get_end_of_line(tokens, start);
        do_preprocessor_include(tokens, defines, include_paths, start, end);
        break;
    }
    default: {
        fprintf(stderr, "%s:%d: todo: Preproccessor %s not handled yet\n", __FILE__, __LINE__, p.str);
        for (int j = 0; j < tokens->count; j++) {
        }
    }
    }
}

void preprocess_file(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths)
{
    for (int i = 0; i < tokens->count; i++) {
        //printf("%s\n", ((token**)tokens->data)[i]->data);
    }
    for (int i = 0; i < tokens->count; i++) {
        token* t = ((token**)tokens->data)[i];
        int key = get_preproccessor(tokens, i);
        
        if (key != -1) {
            for (int j = 0 ; j < defines->count; j++) {
                if (((define**)defines->data)[j] != NULL) {
                }                
            }
            do_preprocessor(tokens, defines, include_paths, i, key);
            
        }
        for (int j = 0; j < defines->count; j++) {
            define* d = ((define**)defines->data)[j];
            if (d != NULL) {
                if (strcmp(d->name->data, t->data) == 0) {
                    replace_token_with_define(tokens, d, i);
                }
            }
        }
        
    }
    for (int i = 0; i < tokens->count; i++) {
        //printf("%s", ((token**)tokens->data)[i]->data);
    }
}


