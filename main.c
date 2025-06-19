#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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


#define DA_CREATE(DA_TYPE)                                          \
    typedef struct {                                                \
        DA_TYPE* data;                                              \
        int count;                                                  \
        int size;                                                   \
    } dynamic_array;                                                \
                                                                    \
    void da_append(dynamic_array* da, DA_TYPE input_data)           \
    {                                                               \
        if (da->count >= da->size) {                                \
            DA_TYPE* new_data = malloc(2*da->size*sizeof(DA_TYPE)); \
            memcpy(new_data, da->data, da->size);                   \
            free(da->data);                                         \
            da->data = new_data;                                    \
            da->size = 2*da->size;                                  \
        }                                                           \
        da->data[da->count] = input_data;                           \
        da->count++;                                                \
    }                                                               \
    void da_init(dynamic_array* da)                                 \
    {                                                               \
        da->size = 256;                                             \
        DA_TYPE* new_data = malloc(da->size*sizeof(DA_TYPE));       \
        da->data = new_data;                                        \
        da->count = 0;                                              \
    }                                                               \


DA_CREATE(char*);
typedef struct {
    dynamic_array* types_list;
} types;

typedef enum {
    TYPE        = 0,
    PARENTHESES = 1,
    SEMICOLON   = 2,
    NUMBER      = 3,
    KEY_WORD    = 4,
    OPERATOR    = 5,
    OTHER       = 6,
} TOKEN_TYPE;

typedef struct {
    TOKEN_TYPE type;
    int pos_in_file;
    char* data;
} token;

bool token_is_type(token* t, types* ts) {
    for (int i = 0; i < ts->types_list->count; i++) {
        if (strcmp(ts->types_list->data[i], t->data) == 0) {
            return true;
        }
    }
    return false;
}

bool token_is_parentheses(token* t) {
    const char parentheses[] = {'(', ')', '{', '}', '[', ']'};
    for (int i = 0; i < sizeof(parentheses)/sizeof(parentheses[0]); i++) {
        if (parentheses[i] == t->data[0]) {
            return true;
        }
    }
    return false;
}

bool token_is_number(token* t) {
    return isdigit(t->data[0]);
}

bool token_is_key_word(token* t) {
    const char* key_words[] = {"auto", "break", "case", "const", "continue", "default", "do", "else", "enum", "extern", "for", "goto", "if", "inline", "register", "restrict", "return", "sizeof", "static", "struct", "switch", "typedef", "typeof", "union", "volatile", "while"};
    for (int i = 0; i < sizeof(key_words)/sizeof(key_words[0]); i++) {
        if (strcmp(key_words[i], t->data) == 0) {
            return true;
        }
    }
    return false;
}


bool token_is_operator(token* t) {
    // https://en.cppreference.com/w/cpp/language/operator_precedence.html
    const char* operators[] = {
        "==", "!=", "<", "<=", ">", ">=", 
        "=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=",
        "+", "-", "*", "/", "%", "&", "|", "^", "<<", ">>", "&&", "||",
        "*", "&", "->", ".", "->*", ".*",
        ",", "?", ":"
    };

    for (int i = 0; i < sizeof(operators)/sizeof(operators[0]); i++) {
        if (strcmp(operators[i], t->data) == 0) {
            return true;
        }
    }
    return false;
}
    
bool token_is_semicolon(token* t) {
    return (';' == t->data[0]);
}

TOKEN_TYPE get_token_type(token* t, types* ts)
{
    if (token_is_type(t, ts)) {
        return TYPE;
    } else if (token_is_parentheses(t)) {
        return PARENTHESES;
    } else if (token_is_semicolon(t)) {
        return SEMICOLON;
    } else if (token_is_number(t)) {
        return NUMBER;
    } else if (token_is_key_word(t)) {
        return KEY_WORD;
    } else if (token_is_operator(t)) {
        return OPERATOR;
    }
    return OTHER;
}

// TODO: This is bad, should have a more robust method of finding the end of a token
bool is_token_end(char c)
{
    return (c == ' ' || c == '\n' || c == ';' || c == '+' || c == '-' || c == '*' || c == '/' || c == '}'|| c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}');
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
            t.pos_in_file = *start_pos - len;
            
        } else {
            free(current_str);
        }
        end_pos++;
    }

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

void generate_default_types(types* ts)
{
    da_append(ts->types_list, "void");
    da_append(ts->types_list, "int");
    da_append(ts->types_list, "bool");
    da_append(ts->types_list, "float");
}

typedef struct AST_node {
    struct AST_node* children;
    int num_children;
    token token;
} AST_node;


void generate_AST(AST_node* root, token tokens[], int* tokens_count)
{
    
}

int main()
{
    char name[] = "./tests/test1.c";
    //char name[] = "./main.c";
    file f;
    store_file(&f, name);
    for (int i = 0; i < f.size; i++) {
        printf("%c", f.data[i]);
    }
    int pos = 0;
    token tokens[10000];
    int tokens_count = 0;
    while (pos < f.size) {
        token t = get_next_token(&f, &pos);
        tokens[tokens_count] = t;
        tokens_count++;
    }

    // Generate default types
    dynamic_array types_da;
    da_init(&types_da);
    types ts = {&types_da};
    generate_default_types(&ts);

    clean_tokens(tokens, &tokens_count);
    for (int i = 0; i < tokens_count; i++) {
        tokens[i].type = get_token_type(&tokens[i], &ts);
        printf("token at %i: %s and type %i\n", tokens[i].pos_in_file, tokens[i].data, tokens[i].type);
    }
    
    // Generate AST
    AST_node* root;
    generate_AST(root, tokens, &tokens_count);

    return 0;
}
