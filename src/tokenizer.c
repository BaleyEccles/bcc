#include "tokenizer.h"

void store_file(file* f, char* file_name)
{
    FILE *input_file;
    
    input_file = fopen(file_name, "r");

    if (input_file == NULL) {
        fprintf(stderr, "%s:%d: error: Unable to open file: %s\n", __FILE__, __LINE__, file_name);
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


bool token_is_type(token* t, dynamic_array ts) {
    for (int i = 0; i < ts.count; i++) {
        if (strcmp(((type*)ts.data)[i].string, t->data) == 0) {
            return true;
        }
    }
    return false;
}

static const mapping parentheses_mapping[] = {
    {"(", PAREN_OPEN},
    {")", PAREN_CLOSE},
    {"{", PAREN_CURLY_OPEN},
    {"}", PAREN_CURLY_CLOSE},
    {"[", PAREN_SQUARE_OPEN},
    {"]", PAREN_SQUARE_CLOSE},
};

bool token_is_parentheses(token* t) {
    for (int i = 0; i < sizeof(parentheses_mapping)/sizeof(parentheses_mapping[0]); i++) {
        if (strcmp(parentheses_mapping[i].string, t->data) == 0) {
            return true;
        }
    }
    return false;
}

TOKEN_TYPE get_token_type_parentheses(token* t) {
    for (int i = 0; i < sizeof(parentheses_mapping)/sizeof(parentheses_mapping[0]); i++) {
        if (strcmp(parentheses_mapping[i].string, t->data) == 0) {
            return parentheses_mapping[i].type;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find parentheses from token with string %s and in position %i\n", __FILE__, __LINE__, t->data, t->pos_in_file);
    return ERROR;
}

// TODO: Add support for floats
bool token_is_number(token* t) {
    return isdigit(t->data[0]);
}


static const mapping key_words_mapping[] = {
    {"auto"     , AUTO},
    {"break"    , BREAK},
    {"case"     , CASE},
    {"const"    , CONST},
    {"continue" , CONTINUE},
    {"default"  , DEFAULT},
    {"do"       , DO},
    {"else"     , ELSE},
    {"enum"     , ENUM},
    {"extern"   , EXTERN},
    {"for"      , FOR},
    {"goto"     , GOTO},
    {"if"       , IF},
    {"inline"   , INLINE},
    {"register" , REGISTER},
    {"restrict" , RESTRICT},
    {"return"   , RETURN},
    {"sizeof"   , SIZEOF},
    {"static"   , STATIC},
    {"struct"   , STRUCT},
    {"switch"   , SWITCH},
    {"typedef"  , TYPEDEF},
    {"typeof"   , TYPEOF},
    {"union"    , UNION},
    {"volatile" , VOLATILE},
    {"while"    , WHILE},
};

TOKEN_TYPE get_token_type_key_word(token* t) {
    for (int i = 0; i < sizeof(key_words_mapping)/sizeof(key_words_mapping[0]); i++) {
        if (strcmp(key_words_mapping[i].string, t->data) == 0) {
            return key_words_mapping[i].type;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find key word from token with string %s and in position %i\n", __FILE__, __LINE__, t->data, t->pos_in_file);
    return ERROR;
}

bool token_is_key_word(token* t) {
    for (int i = 0; i < sizeof(key_words_mapping)/sizeof(key_words_mapping[0]); i++) {
        if (strcmp(key_words_mapping[i].string, t->data) == 0) {
            return true;
        }
    }
    return false;
}






bool token_is_operator(token* t)
{
    for (int i = 0; i < sizeof(operator_mapping)/sizeof(operator_mapping[0]); i++) {
        if (strcmp(operator_mapping[i].string, t->data) == 0) {
            return true;
        }
    }
    return false;
}

TOKEN_TYPE get_token_type_operator(token* t)
{
    for (int i = 0; i < sizeof(operator_mapping)/sizeof(operator_mapping[0]); i++) {
        if (strcmp(operator_mapping[i].string, t->data) == 0) {
            return operator_mapping[i].type;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find operator from token with string %s and in position %i\n", __FILE__, __LINE__, t->data, t->pos_in_file);
    return ERROR;
}
    
bool token_is_semicolon(token* t)
{
    return (';' == t->data[0]);
}

TOKEN_TYPE get_token_type(token* t, dynamic_array* ts)
{
    if (token_is_type(t, *ts)) {
        return TYPE;
    } else if (token_is_parentheses(t)) {
        return get_token_type_parentheses(t);
    } else if (token_is_semicolon(t)) {
        return SEMICOLON;
    } else if (token_is_number(t)) {
        return NUMBER;
    } else if (token_is_key_word(t)) {
        return get_token_type_key_word(t);
    } else if (token_is_operator(t)) {
        return get_token_type_operator(t);
    }
    return OTHER;
}


bool is_token_end(char c)
{
    return (c == ' ' || c == '\n' || c == ';' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == EOF);
}

bool is_token(file* f, char* current_str, int* start_pos, int end_pos)
{
    int len = end_pos - *start_pos;
    if (is_token_end(f->data[end_pos]) || is_token_end(f->data[*start_pos])) {
        *start_pos += len;
        return true;
    }
    

    return false;
}

token* get_next_token(file* f, int* start_pos)
{
    token* t = malloc(sizeof(token));    
    int end_pos = *start_pos + 1;
    bool token_found = false;
    
    while (!token_found) {
        int len = end_pos - *start_pos;
        
        char* current_str = malloc((len + 1)*sizeof(char));
        
        strncpy(current_str, f->data + *start_pos, len);
        current_str[len] = '\0';

        if (is_token(f, current_str, start_pos, end_pos)) {
            token_found = true;
            t->data = current_str;
            t->pos_in_file = *start_pos - len;
            
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
    
void clean_tokens(dynamic_array* tokens)
{
    for (int i = 0; i < tokens->count; i++) {
        remove_bad_chars(((token**)tokens->data)[i]->data);
        
        // remove empty tokens 
        if (strlen(((token**)tokens->data)[i]->data) == 0) {
            for (int j = i; j < tokens->count; j++) {
                ((token**)tokens->data)[j] = ((token**)tokens->data)[j + 1];
            }
            i--;
            (tokens->count)--;
        }
    }
}

type* get_type_from_str(dynamic_array* types, char* str)
{
    for (int i = 0; i < types->count; i++) {
        if (strcmp(((type**)types->data)[i]->string, str) == 0) {
            return ((type**)types->data)[i];
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find type from string, was checking type %s\n", __FILE__, __LINE__, str);
    return NULL;
}

int get_token_location(dynamic_array* tokens, token* t)
{
    for (int i = 0; i < tokens->count; i++) {
        if (((token**)tokens->data)[i] == t) {
            return i;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find token index\n", __FILE__, __LINE__);
    return -1;
}


int find_semi_colon(dynamic_array* tokens, int start_location)
{
    for (int i = start_location; i < tokens->count; i++) {
        if (((token**)tokens->data)[i]->type == SEMICOLON) {
            return i;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find semicolon, staring at %i\n", __FILE__, __LINE__, ((token**)tokens->data)[start_location]->pos_in_file);
    return -1;
}

