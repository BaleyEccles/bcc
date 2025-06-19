#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int size;
    char* data;
} file;


void store_file(file* f, char* file_name)
{
    FILE *input_file;
    
    input_file = fopen(file_name, "r");

    if (input_file == NULL) {
        printf("ERROR: Unable to open file: %s\n", file_name);
    }

    // Get file size
    fseek(input_file, 0L, SEEK_END);
    int size = ftell(input_file);
    rewind(input_file);

    // Read file int file_data
    char* file_data = malloc(sizeof(char)*size);
    int i = 0;
    int c = 1;
    while((c = getc(input_file)) != EOF) {
        file_data[i] = (unsigned char)c;
        i++;
    }
    fclose(input_file);

    f->size = size;
    f->data = file_data;

}

typedef enum {
    TYPE = 0,
    PARENTHESES,
    VARIBLE,
    FUNCTION,
    CONSTANT,
    KEY_WORD,
    UNKNOWN,
} TOKEN_TYPE;

typedef struct {
    TOKEN_TYPE type;
    char* data;
} token;

bool token_is_type(token* t) {
    return true;
}

bool token_is_parentheses(token* t) {
    return true;
}

bool token_is_varible(token* t) {
    return true;
}

bool token_is_function(token* t) {
    return true;
}

bool token_is_constant(token* t) {
    return true;
}

bool token_is_key_word(token* t) {
    return true;
}


TOKEN_TYPE get_token_type(token* t)
{
    if (token_is_type(t)) {
        return TYPE;
    } else if (token_is_parentheses(t)) {
        return PARENTHESES;
    } else if (token_is_varible(t)) {
        return VARIBLE;
    } else if (token_is_function(t)) {
        return FUNCTION;
    } else if (token_is_constant(t)) {
        return CONSTANT;
    } else if (token_is_key_word(t)) {
        return KEY_WORD;
    }
    return UNKNOWN;
}

// TODO: This is bad, should have a more robust method of finding the end of a token
bool is_token_end(char c)
{
    return (c == ' ' || c == ';' || c == '+' || c == '-' || c == '*' || c == '/' || c == '}'|| c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}');
}

bool is_token(file* f, char* current_str, int* start_pos, int end_pos)
{
    int len = end_pos - *start_pos;
    if (is_token_end(f->data[end_pos])) {
        *start_pos += len;
        return true;
    }

    return false;
}

token get_next_token(file* f, int* start_pos)
{
    token t;    
    int end_pos = *start_pos + 1;
    bool token_found = false;
    
    while (!token_found) {
        int len = end_pos - *start_pos;
        
        char* current_str = malloc((len + 1)*sizeof(char));
        
        strncpy(current_str, f->data + *start_pos, len);
        current_str[len] = '\0';

        if (is_token(f, current_str, start_pos, end_pos)) {
            token_found = true;
            t.data = current_str;
            
        } else {
            free(current_str);
        }
        end_pos++;
    }

    t.type = get_token_type(&t);
    return t;
}

void remove_bad_chars(char* data)
{
    
    int len = strlen(data);
    for (int i = 0; i < len; i++) {
        if (data[i] == ' ' || data[i] == '\n') {
            for (int j = i; j < len; j++) {
                data[j] = data[j + 1];
            }
            i--;
            len--;
        }
    }

}
    
void clean_tokens(token tokens[], int* tokens_count)
{
    printf("token count2: %i\n", *tokens_count);
    for (int i = 0; i < *tokens_count; i++) {
        remove_bad_chars(tokens[i].data);
        
        // remove empty tokens 
        if (strlen(tokens[i].data) == 0) {
            for (int j = i; j < *tokens_count; j++) {
                tokens[j] = tokens[j + 1];
            }
            i--;
            (*tokens_count)--;
        }
    }

}

int main()
{
    char name[] = "./tests/test1.c";
    file f;
    store_file(&f, name);
    for (int i = 0; i < f.size; i++) {
        printf("%c", f.data[i]);
    }
    int pos = 1;
    token tokens[1000];
    int tokens_count = 0;
    while (pos < f.size) {
        token t = get_next_token(&f, &pos);
        tokens[tokens_count] = t;
        tokens_count++;
    }
    
    clean_tokens(tokens, &tokens_count);
    for (int i = 0; i < tokens_count; i++) {
        printf("token: %s\n", tokens[i].data);
    }
    

    

    return 0;
}
