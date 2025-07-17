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

void replace_tokens(dynamic_array* tokens, token* t, int start_range, int end_range, int start, int end)
{
    
    int len = end - start;
    printf("len %i\n", len);

    for (int j = 0; j < len; j++) {
        printf("t %s\n", ((token**)tokens->data)[j + start]->data);
    }
    for (int i = start_range; i < end_range; i++) {
        token* current_token = ((token**)tokens->data)[i];
        if (strcmp(current_token->data, t->data) == 0) {
            printf("SHIFT\n");
            da_shift(tokens, token*, i, len - 1);
            

            for (int j = 0; j < len; j++) {
                
                if (start_range < start) {
                    printf("replacing %s with %s\n", ((token**)tokens->data)[j + i]->data, ((token**)tokens->data)[j + start + len - 1]->data);
                    ((token**)tokens->data)[j + i] = ((token**)tokens->data)[j + start + len - 1];

                    //((token**)tokens->data)[j + i]->data = "H ";
                    printf("added H %i\n", j);

                } else {
                    printf("len: %i\n", len);
                    printf("replacing %s with %s\n", ((token**)tokens->data)[j + i]->data, ((token**)tokens->data)[j + start]->data);
                    ((token**)tokens->data)[j + i] = ((token**)tokens->data)[j + start];
                }
            }
            printf("\n");
        }
    }
    
    printf("----------\n");
}

void remove_tokens(dynamic_array* tokens, int start, int end)
{
    token* t = malloc(sizeof(token));
    t->data = " ";
    
    for (int i = start; i < end; i++) {
        ((token**)tokens->data)[i] = t;
    }
}

void replace_tokens_macro(dynamic_array* tokens, token* t, int macro_input_start, int macro_input_end, int start, int end)
{
    int macro_end = get_end_of_define(tokens, macro_input_end);
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

    
    int len = end - start;
    for (int i = start; i < tokens->count; i++) {
        token* current_token = ((token**)tokens->data)[i];
        if (strcmp(current_token->data, t->data) == 0) {
            if (strcmp(((token**)tokens->data)[i + 1]->data, "(") != 0) {
                fprintf(stderr, "%s:%d: error: Next token is not a ( for macro %s, %i\n", __FILE__, __LINE__, current_token->data, current_token->pos_in_file);
            }
            

            int open_paren = i + 1;
            int close_paren = get_closing_paren_location(tokens, open_paren);

            dynamic_array* token_idxs = malloc(sizeof(dynamic_array));
            da_init(token_idxs, int);
            
            for (int j = open_paren + 1; j < close_paren; j++) {
                int end_arg = find_comma(tokens, j, close_paren - 1);
                da_append(token_idxs, j, int);
                da_append(token_idxs, end_arg - 1, int);
                j = end_arg;
            }
            ((int*)token_idxs->data)[token_idxs->count - 1]++;

            printf("loc: %i\n", ((int*)token_idxs->data)[0]);
            replace_tokens(tokens, t, macro_input_end + 1, tokens->count, macro_input_end + 1, macro_end);

            int macro_len = macro_end - macro_input_end;
            for (int k = 0; k < token_idxs->count; k += 2) {
                ((int*)token_idxs->data)[k + 0] += macro_len - 2;
                ((int*)token_idxs->data)[k + 1] += macro_len - 1;
            }
            
            int opening_paren_loc = ((int*)token_idxs->data)[0] - 1;

            for (int i = 0; i < tokens->count; i++) {
                printf("%s", ((token**)tokens->data)[i]->data);
            }
            
            for (int k = 0; k < token_idxs->count; k += 2) {

                int a = ((int*)token_idxs->data)[k];
                int b = ((int*)token_idxs->data)[k + 1];
                
                replace_tokens(tokens, ((token**)macro_inputs->data)[k/2],
                               i - macro_len, ((int*)token_idxs->data)[k],
                               ((int*)token_idxs->data)[k], ((int*)token_idxs->data)[k + 1]);

                // This may fail if k + 1 + 2 is not allocated. This is bad. if k < count do this, or somthing.
                ((int*)token_idxs->data)[k + 0 + 2] += ((int*)token_idxs->data)[k + 1] - ((int*)token_idxs->data)[k];
                ((int*)token_idxs->data)[k + 1 + 2] += ((int*)token_idxs->data)[k + 1] - ((int*)token_idxs->data)[k];
                opening_paren_loc += ((int*)token_idxs->data)[k + 1] - ((int*)token_idxs->data)[k];
            }

            for (int i = 0; i < tokens->count; i++) {
                printf("%s", ((token**)tokens->data)[i]->data);
            }
            int del_start = opening_paren_loc;
            printf("d1 %i\n", del_start);
            int del_end = get_closing_paren_location(tokens, del_start) + 1;
            printf("d1 %i d2 %i\n",del_start, del_end);
            remove_tokens(tokens, del_start, del_end);
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
#define MAX_PREPROCESSOR_SIZE 32
void preprocess_file(dynamic_array* tokens)
{
    for (int i = 0; i < tokens->count; i++) {
        printf("%s", ((token**)tokens->data)[i]->data);
    }
    
    for (int i = 0; i < tokens->count; i++) {
        int key = get_preproccessor(tokens, i);
        if (key != -1) {
            do_preprocessor(tokens, i, key);
            for (int i = 0; i < tokens->count; i++) {
                printf("%s", ((token**)tokens->data)[i]->data);
            }
        }
    }

}


