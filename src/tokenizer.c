#include "tokenizer.h"

bool token_is_type(token* t, dynamic_array* ts) {
    if (ts != NULL) {
        for (int i = 0; i < ts->count; i++) {
            if (strcmp(((type**)ts->data)[i]->string, t->data) == 0) {
                return true;
            }
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
        fprintf(stderr, "%s:%d: error: input to get_closing_paren_location was not an opening parenthese, it was %s at %i with type %i\n", __FILE__, __LINE__, ((token**)tokens->data)[starting_token_location]->data, ((token**)tokens->data)[starting_token_location]->pos_in_file, ((token**)tokens->data)[starting_token_location]->type);
    }
    
    int i = starting_token_location;
    int paren_count = 1;
    TOKEN_TYPE token_type = ((token**)tokens->data)[starting_token_location]->type;
    while (paren_count != 0) {
        i++;
        if (i >= tokens->count) {
            fprintf(stderr, "%s:%d: error: Unable to find closing paren for %s, at index %i and location %i\n", __FILE__, __LINE__, ((token**)tokens->data)[starting_token_location]->data, starting_token_location, ((token**)tokens->data)[starting_token_location]->pos_in_file);
            *(int*)0 = 0;
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


bool token_is_parentheses(token* t) {
    if (t != NULL) {
        for (int i = 0; i < sizeof(parentheses_mapping)/sizeof(parentheses_mapping[0]); i++) {
            if (strcmp(parentheses_mapping[i].string, t->data) == 0) {
                return true;
            }
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

bool token_is_modifier(token* t)
{
    return (t->type == EQUALS        ||
            t->type == PLUS_EQUALS   ||
            t->type == MINUS_EQUALS  ||
            t->type == TIMES_EQUALS  ||
            t->type == DIVIDE_EQUALS ||
            t->type == MODULO_EQUALS);
}

bool token_is_string(token* t)
{
    return t->data[0] == '"';

}

// TODO: Add support for floats
bool token_is_number(token* t)
{
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

int get_ptr_count(dynamic_array* tokens, token* t)
{
    
    int loc = get_token_location(tokens, t) + 1;
    get_token_type(NULL, tokens, ((token**)tokens->data)[loc]);
    int ptr_count = 0;
    while (((token**)tokens->data)[loc]->type == TIMES) {
        loc++;
        ptr_count++;
        get_token_type(NULL, tokens, ((token**)tokens->data)[loc]);
    }
        
    return ptr_count;
}

bool is_type_defined(dynamic_array* ts, token* t, int ptr_count)
{
    for (int i = 0; i < ts->count; i++) {
        type* ty = ((type**)ts->data)[i];
        if (strcmp(ty->string, t->data) == 0 && ty->ptr_count == ptr_count) {
            return true;
        }
    }
    return false;
}

type* get_type(dynamic_array* tokens, dynamic_array* types, token* t)
{
    int ptr_count = get_ptr_count(tokens, t);
    
    for (int i = 0; i < types->count; i++) {
        type* ty = ((type**)types->data)[i];
        if (strcmp(ty->string, t->data) == 0 && ty->ptr_count == ptr_count) {
            return ty;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find type from token %s in posistion %i \n", __FILE__, __LINE__, t->data, t->pos_in_file);
    return NULL;
}

void generate_type(dynamic_array* ts, dynamic_array* tokens, token* t)
{
    int ptr_count = get_ptr_count(tokens, t);
    if (!is_type_defined(ts, t, ptr_count)) {
        type* ty = malloc(sizeof(type));
        ty->ptr_count = ptr_count;
        ty->string = t->data;
        if (ptr_count > 0) {
            ty->size = 8;
        } else {
            fprintf(stderr, "%s:%d: TODO: type size was not defined\n", __FILE__, __LINE__);
        }
        da_append(ts, ty, type*);
    }
}

void get_token_type(dynamic_array* ts, dynamic_array* tokens, token* t)
{
    if (token_is_type(t, ts)) {
        generate_type(ts, tokens, t);
        t->type = TYPE;
    } else if (token_is_parentheses(t)) {
        t->type = get_token_type_parentheses(t);
    } else if (token_is_operator(t)) {
        t->type = get_token_type_operator(t);
    } else if (token_is_semicolon(t)) {
        t->type = SEMICOLON;
    } else if (token_is_number(t)) {
        t->type = NUMBER;
    } else if (token_is_key_word(t)) {
        t->type = get_token_type_key_word(t);
    } else {
        t->type = OTHER;
    }
}


bool is_token_end(char* str, char c_next)
{
    bool is_end = false;
    if (str[0] == '"') {
        int len = strlen(str) - 1;
        if (len > 1 && str[len] == '"') {
            
            is_end = true;
        }
    } else {
        int size = strlen(str);
        char c_start = str[0];
        is_end =
            c_next == ' '  ||
            c_next == '\t' ||
            c_next == '\n' ||
            c_next == ';'  ||
            c_next == '}'  ||
            c_next == '('  ||
            c_next == ')'  ||
            c_next == '['  ||
            c_next == ']'  ||
            c_next == '{'  ||
            c_next == '}'  ||
            c_next == ','  ||
            c_next == '*'  ||
            c_next == '#'  ||
            c_next == '!'  ||
            c_next == EOF;
        is_end =
            (c_start == ' '  ||
             c_start == '\t' ||
             c_start == '\n' ||
             c_start == ';'  ||
             c_start == '}'  ||
             c_start == '('  ||
             c_start == ')'  ||
             c_start == '['  ||
             c_start == ']'  ||
             c_start == '{'  ||
             c_start == '}'  ||
             c_start == ','  ||
             c_start == '#'  ||
             c_start == '!'  ||
             c_start == EOF) ||
            is_end;
        if (str[0] == '<' && str[strlen(str) - 1] == '-') {
            is_end = false;
        }
        if ((c_next == '+' || c_next == '-') && isalpha(str[size - 1])) {
            is_end = true;
        }
    }
    return is_end;
}

bool is_token(FILE* f, char* current_str, int* start_pos, int end_pos)
{
    int len = end_pos - *start_pos;
    if (fseek(f, end_pos, SEEK_SET) != 0) {
        fprintf(stderr, "%s:%d: error: fseek failed\n", __FILE__, __LINE__);
        return 1;
    }
    char c_next = getc(f);
    rewind(f);
    
    if (is_token_end(current_str, c_next)) {
        *start_pos += len;
        return true;
    }
    

    return false;
}

token* get_next_token(FILE* f, int* start_pos)
{
    token* t = malloc(sizeof(token));
    int end_pos = *start_pos + 1;
    bool token_found = false;
    
    while (!token_found) {
        int len = end_pos - *start_pos;
        
        char* current_str = malloc((len + 1)*sizeof(char));
        fseek(f, *start_pos, SEEK_SET);
        fread(current_str, sizeof(char), len, f);
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
    if (data[0] != '"') {
        int len = strlen(data);
        for (int i = 0; i < len; i++) {
            if (data[i] == ' ' || data[i] == '\t' || data[i] == '\n') {
                for (int j = i; j < len; j++) {
                    data[j] = data[j + 1];
                }
                i--;
                len--;
            }
        }
    }
}



void untabbify_tokens(dynamic_array* tokens)
{

    for (int i = 0; i < tokens->count; i++) {
        token* t = ((token**)tokens->data)[i];
        if (strcmp(t->data, "\t") == 0) {
            ALLOC_STR(t, " ");
        }
    }
}

void clean_tokens(dynamic_array* tokens)
{
    untabbify_tokens(tokens);
    for (int i = 0; i < tokens->count; i++) {
        token* t = ((token**)tokens->data)[i];
        if (t->data == NULL) {
            ALLOC_STR(t, "");
        }

        if (strcmp(t->data, "\\") == 0 && strcmp(((token**)tokens->data)[i + 1]->data, "\n") == 0) {
            ALLOC_STR(t, "");
        }
        remove_bad_chars(t->data);
        // remove empty tokens 
        if (strlen(t->data) == 0) {
            for (int j = i; j < tokens->count; j++) {
                ((token**)tokens->data)[j] = ((token**)tokens->data)[j + 1];
            }
            i--;
            (tokens->count)--;
        }
    }
}

void remove_comments(dynamic_array* tokens)
{
    for (int i = 0; i < tokens->count; i++) {
        token* t = ((token**)tokens->data)[i];
        if (strcmp(t->data, "//") == 0) {
            while (strcmp(t->data, "\n") != 0) {
                ALLOC_STR(t, "");
                i++;
                t = ((token**)tokens->data)[i];
            }
        }
        if (strcmp(t->data, "/") == 0 && strcmp(((token**)tokens->data)[i + 1]->data, "*") == 0 ) {
            while (strcmp(t->data, "*/") != 0) {
                ALLOC_STR(t, "");
                i++;
                t = ((token**)tokens->data)[i];
            }
            ALLOC_STR(t, "");
        }
    }
}



type* get_type_from_str(dynamic_array* types, char* str)
{
    fprintf(stderr, "%s:%d: error: USING OBSOLETE FUNCTION %s\n", __FILE__, __LINE__, str);
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

int find_comma(dynamic_array* tokens, int start, int end)
{
    for (int i = start; i < end + 1; i++) {
        token* t = ((token**)tokens->data)[i];
        if (t != NULL) {
            if (token_is_parentheses(t)) {
                i = get_closing_paren_location(tokens, i);
            } else if (strcmp(t->data, ",") == 0) {
                return i;
            }
        }
    }
    return end;
}

type* get_number_type(dynamic_array* types, token* t)
{
    if (types == NULL) {
        return NULL;
    }
    for (int i = 0; i < types->count; i++) {
        type* ty = ((type**)types->data)[i];
        // TOOD: other types than int, like float/double
        if (strcmp(ty->string, "int") == 0 && ty->ptr_count == 0) {
            return ty;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find number type for %s, %i type\n", __FILE__, __LINE__, t->data, t->pos_in_file);
    return NULL;
}

type* get_string_type(dynamic_array* types)
{
    for (int i = 0; i < types->count; i++) {
        type* ty = ((type**)types->data)[i];
        if (strcmp(ty->string, "char") == 0 && ty->ptr_count == 1) {
            return ty;
        }
    }
    fprintf(stderr, "%s:%d: error: Unable to find string type\n", __FILE__, __LINE__);
    return NULL;
}

bool type_is_string(type* ty)
{
    return (strcmp(ty->string, "char") == 0 && ty->ptr_count == 1);
}

type* get_type_from_name_and_ptr_count(dynamic_array* types, char* str, int ptr_count)
{
    for (int i = 0; i < types->count; i++) {
        type* ty = ((type**)types->data)[i];
        if (ty->ptr_count == ptr_count && strcmp(ty->string, str) == 0) {
            return ty;
        }
    }
    return NULL;
}

type* get_dereferenced_type(dynamic_array* types, type* t)
{
    type* ty = get_type_from_name_and_ptr_count(types, t->string, t->ptr_count - 1);
    if (ty == NULL) {
        ty = malloc(sizeof(type));
        ty->ptr_count = t->ptr_count - 1;
        ty->string = t->string;
        da_append(types, ty, type*);
    }
    return ty;
}
