#include "preprocessor.h"

typedef struct {
    PREPROCESSORS t;
    char* str;
} str_enum_map;

#define PREPROCESSORS_COUNT 9
static const str_enum_map preprocessors[] = {
    {PRE_PROCESS_DEFINE, "define"}, {PRE_PROCESS_INCLUDE, "include"}, {PRE_PROCESS_IF, "if"}, {PRE_PROCESS_ELSE, "else"}, {PRE_PROCESS_ELIF, "elif"}, {PRE_PROCESS_IFDEF, "ifdef"}, {PRE_PROCESS_IFNDEF, "ifndef"}, {PRE_PROCESS_ENDIF, "endif"}, {PRE_PROCESS_UNDEF, "undef"}
};


PREPROCESSORS get_preproccessor(dynamic_array* tokens, int location)
{
    token* t1 = ((token**)tokens->data)[location];
    token* t2;
    for (int i = location + 1; i < tokens->count; i++) {
        t2 = ((token**)tokens->data)[i];
        if (strcmp(t2->data, " ") != 0) {
            break;
        }
    }

    for (int i = 0; i < PREPROCESSORS_COUNT; i++) {
        char* type = preprocessors[i].str;
        if (strcmp(t2->data, type) == 0 && strcmp(t1->data, "#") == 0) {
            return i;
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
                ((token**)tokens_dst->data)[j + i] = ((token**)tokens_src->data)[j];
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
                ((token**)tokens->data)[j + i] = ((token**)tokens->data)[j + start];
                
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
    t->data = "";
    
    for (int i = start; i < end; i++) {
        ((token**)tokens->data)[i] = t;
    }
}

void create_define(dynamic_array* tokens, define* d, int start, int end)
{
    d->inputs = NULL;
    
    dynamic_array* define_output = malloc(sizeof(dynamic_array));
    da_init(define_output, token*);    
    for (int i = start; i < end; i++) {
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
    for (int i = macro_input_end + 1; i < end; i++) {
        da_append(macro_output, ((token**)tokens->data)[i], token*);
    }
    d->output = macro_output;
}

char* get_path(dynamic_array* paths, char* name) {
    for (int i = 0; i < paths->count; i++) {
        char* str = ((char**)paths->data)[i];
        char* path = malloc(sizeof(char)*(strlen(name) + strlen(str) + 1));
        for (int j = 0; j < strlen(str); j++) {
            path[j] = str[j];
        }
        path[strlen(str)] = '/';
        for (int j = strlen(str) + 1; j < strlen(name) + strlen(str) + 1; j++) {
            path[j] = name[j - strlen(str) - 1];
        }


        if (access(path, F_OK) == 0) {
            return path;
        }
        free(path);
    }
    fprintf(stderr, "%s:%d: error: Unable to find file %s\n", __FILE__, __LINE__, name);
    return "";
}

int find_else(dynamic_array* tokens, int start)
{
    // TODO: similar to find_endif
    return -1;
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

void do_preprocessor_ifndef(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int start, int end)
{

    token* ifndef_token;
    int ifndef_token_loc;
    for (int i = start; i < end; i++) {
        token* t = ((token**)tokens->data)[i];
        if (strcmp(t->data, "ifndef") == 0) {
            ifndef_token = t;
            ifndef_token_loc = i;
            break;
        }
    }

    token* define_token;
    int define_token_loc;
    for (int i = ifndef_token_loc + 1; i < end; i++) {
        token* t1 = ((token**)tokens->data)[i - 1];
        token* t2 = ((token**)tokens->data)[i + 0];
        if (strcmp(t1->data, " ") == 0 && strcmp(t2->data, " ") != 0) {
            define_token = t2;
            define_token_loc = i;
            break;
        }
    }
    
    int endif_token_loc = find_endif(tokens, ifndef_token_loc + 1);
    token* endif_token = ((token**)tokens->data)[endif_token_loc];
    
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
    if (defined) {
        remove_tokens(tokens, start, endif_token_loc + 1);
    } else {
        remove_tokens(tokens, start, end);
        int endif_hash_loc = -1;
        for (int i = endif_token_loc; i >= start; i--) {
            token* t = ((token**)tokens->data)[i];
            if (strcmp(t->data, "#") == 0) {
                endif_hash_loc = i;
                break;
            }
        }
        remove_tokens(tokens, endif_hash_loc, endif_token_loc + 1);
    }
}
// GIGA repetition above and below
void do_preprocessor_ifdef(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int start, int end)
{

    token* ifdef_token;
    int ifdef_token_loc;
    for (int i = start; i < end; i++) {
        token* t = ((token**)tokens->data)[i];
        if (strcmp(t->data, "ifdef") == 0) {
            ifdef_token = t;
            ifdef_token_loc = i;
            break;
        }
    }

    token* define_token;
    int define_token_loc;
    for (int i = ifdef_token_loc + 1; i < end; i++) {
        token* t1 = ((token**)tokens->data)[i - 1];
        token* t2 = ((token**)tokens->data)[i + 0];
        if (strcmp(t1->data, " ") == 0 && strcmp(t2->data, " ") != 0) {
            define_token = t2;
            define_token_loc = i;
            break;
        }
    }
    
    int endif_token_loc = find_endif(tokens, ifdef_token_loc + 1);
    token* endif_token = ((token**)tokens->data)[endif_token_loc];
    
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
    if (!defined) {
        remove_tokens(tokens, start, endif_token_loc + 1);
    } else {
        remove_tokens(tokens, start, end);
        int endif_hash_loc = -1;
        for (int i = endif_token_loc; i >= start; i--) {
            token* t = ((token**)tokens->data)[i];
            if (strcmp(t->data, "#") == 0) {
                endif_hash_loc = i;
                break;
            }
        }
        remove_tokens(tokens, endif_hash_loc, endif_token_loc + 1);
    }
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
            int end_defined = loc;
            token* t2 = ((token**)ts->data)[loc];
            if (strcmp(t2->data, "(") == 0) {
                loc += 1;
                end_defined = loc + 1;
                t2 = ((token**)ts->data)[loc];
            }
            bool defined = false;
            for (int j = 0; j < defines->count; j++) {
                define* d = ((define**)defines->data)[j];
                if (d != NULL) {
                    if (strcmp(t2->data, d->name->data) == 0) {

                        defined = true;
                        break;
                    }
                }
            }
            dynamic_array* da = malloc(sizeof(dynamic_array));
            da_init(da, token*);
            if (defined) {
                token* token_defined = malloc(sizeof(token));
                token_defined->data = "1";
                da_append(da, token_defined, token*);

            } else {
                token* token_defined = malloc(sizeof(token));
                token_defined->data = "0";
                da_append(da, token_defined, token*);
            }
            
            replace_tokens_with_array(ts, t2, loc, end_defined, da);
            remove_tokens(ts, i, i + 1);
            
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
    
    context ctx = {
        ts,
        NULL,
        NULL
    };

    AST_node* n = create_expression_node(NULL, &ctx, 0, ts->count - 1);
    return evaluate_node(n);

}

void do_preprocessor_if(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int start, int end)
{
    token* if_token;
    int if_token_loc;
    for (int i = start; i < end; i++) {
        token* t = ((token**)tokens->data)[i];
        if (strcmp(t->data, "if") == 0) {
            if_token = t;
            if_token_loc = i;
            break;
        }
    }
    
    int endif_token_loc = find_endif(tokens, if_token_loc + 1);
    token* endif_token = ((token**)tokens->data)[endif_token_loc];

    int val = evaluate_expression(tokens, defines, if_token_loc + 1, end);
    if (val == 0) {
    }
}



void do_preprocessor_include(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths, int start, int end)
{
    char* include_name;
    token* include_token;
    for (int i = start; i < end; i++) {
        token* t = ((token**)tokens->data)[i];
        if ((t->data[0] == '<' && t->data[strlen(t->data) - 1] == '>') ||
            (t->data[0] == '"' && t->data[strlen(t->data) - 1] == '"')) {
            include_name = malloc((strlen(t->data) - 2)*sizeof(char));
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
            include_name = malloc(size*sizeof(char));
            size = 0;
            for (int j = 0; j < count; j++) {
                t = ((token**)tokens->data)[idx + j];
                strcpy(include_name + size*sizeof(char), t->data);
                size += strlen(t->data);
            }
            break;
        }
    }
    
    bool external_file = false;
    if (include_name[0] == '<') {
        external_file = true;
    }
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


    FILE* file = fopen(file_name, "r");
    fseek(file, 0L, SEEK_END);
    int file_size = ftell(file);
    rewind(file);

    dynamic_array file_tokens;
    da_init(&file_tokens, token*);
    
    int pos = 0;
    while (pos < file_size) {
        token* t = get_next_token(file, &pos);
        da_append(&file_tokens, t, token*);
    }
    untabbify_tokens(&file_tokens);
    remove_comments(&file_tokens);
    preprocess_file(&file_tokens, defines, include_paths);
    replace_tokens_with_array(tokens, include_token, start, end, &file_tokens);
    
}

void do_preprocessor_undefine(dynamic_array* tokens, dynamic_array* defines, int start, int end)
{
    token* undef;
    int i = start;
    for (i = start; i < end; i++) {
        undef = ((token**)tokens->data)[i];
        if (strcmp(undef->data, "undef") == 0) {
            break;
        }
    }
    
    token* name;
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
    } else {
        replace_tokens_with_array(tokens, current_token, loc, loc + 1, d->output); // Replace define name with define output
    }
}

void do_preprocessor_define(dynamic_array* tokens, dynamic_array* defines, int start, int end)
{
    // We need to know if there is a space between MACRO and (.
    // This will determine if it is a function macro, or a simple find and replace
    // #define MACRO(a, b) a + b
    // ^start                  ^end
    token* type;
    int i;
    for (i = start + 1; i < tokens->count; i++) {
        type = ((token**)tokens->data)[i];
        if (strcmp(type->data, " ") != 0) {
            break;
        }
    }
    token* name;
    for (i = i + 1; i < tokens->count; i++) {
        name = ((token**)tokens->data)[i];
        if (strcmp(name->data, " ") != 0) {
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
    case PRE_PROCESS_INCLUDE: {
        int end = get_end_of_line(tokens, start);
        do_preprocessor_include(tokens, defines, include_paths, start, end);
        break;
    }
    default: {
        fprintf(stderr, "%s:%d: todo: Preproccessor %s not handled yet\n", __FILE__, __LINE__, p.str);
    }
    }
}

void preprocess_file(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths)
{

    for (int i = 0; i < tokens->count; i++) {
        token* t = ((token**)tokens->data)[i];
        int key = get_preproccessor(tokens, i);
        if (key != -1) {
            
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
        printf("%s", ((token**)tokens->data)[i]->data);
    }
}


