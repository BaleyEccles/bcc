#include "preprocessor.h"

typedef enum {
    PRE_PROCESS_DEFINE,
    PRE_PROCESS_INCLUDE,
    PRE_PROCESS_IF,
    PRE_PROCESS_ELSE,
    PRE_PROCESS_ELIF,
    PRE_PROCESS_IFDEF,
    PRE_PROCESS_IFNDEF,
    PRE_PROCESS_ENDIF,
    PRE_PROCESS_UNDEF,
} PREPROCESSORS;

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

void replace_tokens_macro(dynamic_array* tokens, token* t, int macro_input_start, int macro_input_end, int start, int end)
{
    dynamic_array* macro_inputs = malloc(sizeof(dynamic_array));
    da_init(macro_inputs, token*);
    for (int i = macro_input_start; i < macro_input_end; i++) {
        da_append(macro_inputs, ((token**)tokens->data)[i], token*);
    }
    clean_tokens(macro_inputs);
    // macro_inputs:
    // [a] [,] [b] [,] [c] ...


    // Remove commas
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

    
    for (int i = start; i < tokens->count; i++) {
        token* current_token = ((token**)tokens->data)[i];
        if (strcmp(current_token->data, t->data) == 0) {
            if (strcmp(((token**)tokens->data)[i + 1]->data, "(") != 0) {
                fprintf(stderr, "%s:%d: error: Next token is not a ( for macro %s, %i\n", __FILE__, __LINE__, current_token->data, current_token->pos_in_file);
            }
            dynamic_array* inputs = malloc(sizeof(dynamic_array));
            da_init(inputs, dynamic_array*);
            int loc = i + 2;
            int macro_end = get_closing_paren_location(tokens, loc - 1);
            while (loc < macro_end) {
                dynamic_array* input = malloc(sizeof(dynamic_array));
                da_init(input, token*);
                int comma_loc = find_comma(tokens, loc, macro_end - 1);
                if (token_is_parentheses(((token**)tokens->data)[comma_loc + 1])) {
                    comma_loc++;
                }
                                                                               
                for (int j = loc; j < comma_loc; j++) {
                    da_append(input, ((token**)tokens->data)[j], token*);
                }
                da_append(inputs, input, dynamic_array*);
                loc = comma_loc + 1;
            }

            if (inputs->count != macro_inputs->count) {
                fprintf(stderr, "%s:%d: error: Mismatched input counts for macro %s %i with %i inputs and %s %i with %i inputs\n", __FILE__, __LINE__, t->data, t->pos_in_file, macro_inputs->count, current_token->data, current_token->pos_in_file, inputs->count);
            }

            // We now have a mapping from definition inputs to application inputs
            // a -> [1 + 2]
            // b -> [5]
            // ...
            remove_tokens(tokens, i + 1, macro_end + 1);
            replace_tokens(tokens, t, i, i + 1, start, end);
            for (int j = 0; j < macro_inputs->count; j++) {
                replace_tokens_with_array(tokens, ((token**)macro_inputs->data)[j], i, macro_end, ((dynamic_array**)inputs->data)[j]);
                macro_end += ((dynamic_array**)inputs->data)[j]->count;
            }

            
            
        }
    }
    
}


void do_preprocessor_define(dynamic_array* tokens, int start, int end)
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

    bool is_macro = false;
    token* next = ((token**)tokens->data)[i + 1];
    if (token_is_parentheses(next)) {
        is_macro = true;
    }
    
    if (is_macro) {
        int macro_inputs_start = i + 1;
        int macro_inputs_end = get_closing_paren_location(tokens, i + 1);
        replace_tokens_macro(tokens, name, macro_inputs_start + 1, macro_inputs_end, macro_inputs_end + 1, end);
        remove_tokens(tokens, start, end);
    } else {
        replace_tokens(tokens, name, i + 2, tokens->count, i + 2, end + 1);
        remove_tokens(tokens, start, end);
    }
    
}

void do_preprocessor(dynamic_array* tokens, int start, int key)
{
    str_enum_map p = preprocessors[key];
    switch (p.t) {
    case PRE_PROCESS_DEFINE: {
        int end = get_end_of_define(tokens, start);
        do_preprocessor_define(tokens, start, end);
        break;
    }
    default: {
        fprintf(stderr, "%s:%d: todo: Preproccessor %s not handled yet\n", __FILE__, __LINE__, p.str);
    }
    }
}

void preprocess_file(dynamic_array* tokens)
{
    
    for (int i = 0; i < tokens->count; i++) {
        int key = get_preproccessor(tokens, i);
        if (key != -1) {
            do_preprocessor(tokens, i, key);
        }
    }

}


