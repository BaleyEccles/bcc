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



typedef struct {                            
    void* data;
    int count;
    int size;
} dynamic_array;
                                         
    
#define da_append(DA, INPUT_DATA, DA_TYPE)                               \
    {                                                                   \
        if ((DA)->count >= (DA)->size) {                                \
            DA_TYPE* new_data = malloc(2 * (DA)->size * sizeof(DA_TYPE)); \
            memcpy(new_data, (DA)->data, (DA)->size * sizeof(DA_TYPE)); \
            free((DA)->data);                                           \
            (DA)->data = new_data;                                      \
            (DA)->size = 2 * (DA)->size;                                \
        }                                                               \
        ((DA_TYPE*)(DA)->data)[(DA)->count] = (INPUT_DATA);       \
        (DA)->count++;                                                  \
    }                                                                   

#define da_init(DA, DA_TYPE)                                            \
    {                                                                   \
        (DA)->size = 256;                                               \
        DA_TYPE* new_data = malloc((DA)->size * sizeof(DA_TYPE));       \
        (DA)->data = new_data;                                          \
        (DA)->count = 0;                                                \
    }


typedef struct {
    char* string;
} type;


typedef enum {
    ERROR = 0,
    TYPE,
    PARENTHESES,
    SEMICOLON,
    NUMBER,
    KEY_WORD,
    OPERATOR,
    OTHER,

    PAREN_OPEN,
    PAREN_CLOSE,
    PAREN_CURLY_OPEN,
    PAREN_CURLY_CLOSE,
    PAREN_SQUARE_OPEN,
    PAREN_SQUARE_CLOSE,

    AUTO,
    BREAK,
    CASE,
    CONST,
    CONTINUE,
    DEFAULT,
    DO,
    ELSE,
    ENUM,
    EXTERN,
    FOR,
    GOTO,
    IF,
    INLINE,
    REGISTER,
    RESTRICT,
    RETURN,
    SIZEOF,
    STATIC,
    STRUCT,
    SWITCH,
    TYPEDEF,
    TYPEOF,
    UNION,
    VOLATILE,
    WHILE,

    BOOL_EQUALS,
    BOOL_NOT_EQUALS,
    BOOL_LESS_THAN,
    BOOL_LESS_THAN_OR_EQUALS,
    BOOL_GREATER_THAN,
    BOOL_GREATER_THAN_OR_EQUALS,
    EQUALS,
    PLUS_EQUALS,
    MINUS_EQUALS,
    TIMES_EQUALS,
    DIVIDE_EQUALS,
    MODULO_EQUALS,
    AND_EQUALS,
    OR_EQUALS,
    XOR_EQUALS,
    LEFT_SHIFT_EQUALS,
    RIGHT_SHIFT_EQUALS,
    PLUS,
    MINUS,
    TIMES,
    DIVIDE,
    MODULO,
    BITWISE_AND,
    BITWISE_OR,
    BITWISE_XOR,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    LOGICAL_AND,
    LOGICAL_OR,
    POINTER_DEREFERENCE,
    ADDRESS_OF,
    MEMBER_ACCESS_POINTER,
    MEMBER_ACCESS,
    MEMBER_DEREFERENCE_POINTER,
    MEMBER_DEREFERENCE,
    TERNARY_CONDITIONAL,
    COLON,
} TOKEN_TYPE;

typedef struct {
    TOKEN_TYPE type;
    int pos_in_file;
    char* data;
} token;

typedef struct {
    char* string;
    TOKEN_TYPE type;
} mapping;


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
    fprintf(stderr, "ERROR: Unable to find parentheses from token with string %s and in position %i\n", t->data, t->pos_in_file);
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
    fprintf(stderr, "ERROR: Unable to find key word from token with string %s and in position %i\n", t->data, t->pos_in_file);
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



// https://en.cppreference.com/w/cpp/language/operator_precedence.html
static const mapping operator_mapping[] = {
    {"=="  , BOOL_EQUALS},
    {"!="  , BOOL_NOT_EQUALS},
    {"<"   , BOOL_LESS_THAN},
    {"<="  , BOOL_LESS_THAN_OR_EQUALS},
    {">"   , BOOL_GREATER_THAN},
    {">="  , BOOL_GREATER_THAN_OR_EQUALS},
    {"="   , EQUALS},
    {"+="  , PLUS_EQUALS},
    {"-="  , MINUS_EQUALS},
    {"*="  , TIMES_EQUALS},
    {"/="  , DIVIDE_EQUALS},
    {"%="  , MODULO_EQUALS},
    {"&="  , AND_EQUALS},
    {"|="  , OR_EQUALS},
    {"^="  , XOR_EQUALS},
    {"<<=" , LEFT_SHIFT_EQUALS},
    {">>=" , RIGHT_SHIFT_EQUALS},
    {"+"   , PLUS},
    {"-"   , MINUS},
    {"*"   , TIMES},
    {"/"   , DIVIDE},
    {"%"   , MODULO},
    {"&"   , BITWISE_AND},
    {"|"   , BITWISE_OR},
    {"^"   , BITWISE_XOR},
    {"<<"  , LEFT_SHIFT},
    {">>"  , RIGHT_SHIFT},
    {"&&"  , LOGICAL_AND},
    {"||"  , LOGICAL_OR},
    {"*"   , POINTER_DEREFERENCE},
    {"&"   , ADDRESS_OF},
    {"->"  , MEMBER_ACCESS_POINTER},
    {"."   , MEMBER_ACCESS},
    {"->*" , MEMBER_DEREFERENCE_POINTER},
    {".*"  , MEMBER_DEREFERENCE},
    {"?"   , TERNARY_CONDITIONAL},
    {":"   , COLON},
};


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
    fprintf(stderr, "ERROR: Unable to find operator from token with string %s and in position %i\n", t->data, t->pos_in_file);
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

void generate_default_types(dynamic_array* ts)
{
    // TODO more types
    type* type_void = malloc(sizeof(type));
    type_void->string = "void";
    da_append(ts, type_void, type*);
    type* type_int = malloc(sizeof(type));
    type_int->string = "int";
    da_append(ts, type_int, type*);
    type* type_bool = malloc(sizeof(type));
    type_bool->string = "bool";
    da_append(ts, type_bool, type*);
    type* type_float = malloc(sizeof(type));
    type_float->string = "float";
    da_append(ts, type_float, type*);
}

typedef enum {
    FUNCTION = 0,
    VARIBLE  = 1,    
} NODE_TYPE;


typedef struct AST_node {
    dynamic_array* children;
    NODE_TYPE node_type;
    token* token;
    void* data;
} AST_node;

void init_AST_node(AST_node* node)
{
    dynamic_array* node_da = malloc(sizeof(dynamic_array));
    da_init(node_da, AST_node);
    node->children = node_da;
}


typedef struct {
    type* type;
    char* name;
} function_input;

typedef struct {
    type* return_type;
    char* name;
    dynamic_array* inputs;
} function;

typedef struct {
    type* type;
    char* value;
} constant;

typedef struct {
    type* type;
    char* name;
} varible;

type* get_type_from_str(dynamic_array* types, char* str)
{
    for (int i = 0; i < types->count; i++) {
        if (strcmp(((type**)types->data)[i]->string, str) == 0) {
            return ((type**)types->data)[i];
        }
    }
    fprintf(stderr, "ERROR: Unable to find type from string, was checking type %s\n", str);
    return NULL;
}

AST_node* get_main_function(dynamic_array* types, dynamic_array* tokens)
{
    AST_node* main_node = malloc(sizeof(AST_node));

    for (int i = 1; i < tokens->count - 1; i++) {
        if (((token**)tokens->data)[i]->type == OTHER &&
            strcmp(((token**)tokens->data)[i]->data, "main") == 0 &&
            ((token**)tokens->data)[i-1]->type == OTHER &&
            ((token**)tokens->data)[i+1]->type == PAREN_OPEN
            ) {
            
            printf("main function on posistion %i\n", ((token**)tokens->data)[i]->pos_in_file);
            
            function* main_function = malloc(sizeof(function));
            main_function->return_type = get_type_from_str(types, ((token**)tokens->data)[i - 1]->data);
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
        fprintf(stderr, "ERROR: Unable to find main function\n");
    }

    return main_node;
}

int find_closing_paren(dynamic_array* tokens, int starting_token_location)
{
    int i = 0;
    int paren_count = 1;
    TOKEN_TYPE token_type = ((token**)tokens->data)[starting_token_location]->type;
    while (paren_count != 0) {
        i++;
        switch (token_type) {
        case PAREN_OPEN: {
            if (((token**)tokens->data)[starting_token_location + i]->type == PAREN_CLOSE) {
                paren_count--;
            } else if (((token**)tokens->data)[starting_token_location + i]->type == PAREN_OPEN) {
                paren_count++;
            }
            break;
        }
        case PAREN_CURLY_OPEN: {
            if (((token**)tokens->data)[starting_token_location + i]->type == PAREN_CURLY_CLOSE) {
                paren_count--;
            } else if (((token**)tokens->data)[starting_token_location + i]->type == PAREN_CURLY_OPEN) {
                paren_count++;
            }
            break;
        }
        case PAREN_SQUARE_OPEN: {
            if (((token**)tokens->data)[starting_token_location + i]->type == PAREN_SQUARE_CLOSE) {
                paren_count--;
            } else if (((token**)tokens->data)[starting_token_location + i]->type == PAREN_SQUARE_OPEN) {
                paren_count++;
            }
            break;
        }
        default: {
            fprintf(stderr, "ERROR: input to find_closing_paren was not a parenthese, it was %s with type %i\n", ((token**)tokens->data)[starting_token_location]->data, ((token**)tokens->data)[starting_token_location]->type);
            break;
        }
        }
    }
    return i;
}

int get_token_location(dynamic_array* tokens, token* t)
{
    for (int i = 0; i < tokens->count; i++) {
        if (((token**)tokens->data)[i] == t) {
            return i;
        }
    }
    fprintf(stderr, "ERROR: Unable to find token index\n");
    return -1;
}

int find_semi_colon(dynamic_array* tokens, int start_location)
{
    for (int i = start_location; i < tokens->count; i++) {
        if (((token**)tokens->data)[i]->type == SEMICOLON) {
            return i;
        }
    }
    fprintf(stderr, "ERROR: Unable to find semicolon, staring at %i\n", ((token**)tokens->data)[start_location]->pos_in_file);
    return -1;
}

AST_node* create_rhs_AST_node(dynamic_array* types, dynamic_array* tokens, int rhs_start_location)
{
    AST_node* rhs_AST_node = malloc(sizeof(AST_node));
    init_AST_node(rhs_AST_node);
    
    int rhs_end_location = find_semi_colon(tokens, rhs_start_location);
    for (int i = rhs_start_location; i < rhs_end_location; i++) {
        // (3)
        // (2) + 3
        // (2*3) + 2
        // 2*(3) + 2
        // 2*(3) + 2
        // 2*(2 + 3) + 2
        if (token_is_number(((token**)tokens->data)[i])) {
            //reate_constant_AST_node()
        }

                
    }
    return NULL;
}

void generate_AST(AST_node* root, dynamic_array* types, dynamic_array* tokens)
{
    root = get_main_function(types, tokens);
    int main_token_location = get_token_location(tokens, root->token);
    int main_inputs_token_end = find_closing_paren(tokens, main_token_location + 1);
    int main_lower_token_range = find_closing_paren(tokens, main_inputs_token_end + 1);
    int main_upper_token_range = find_closing_paren(tokens, main_lower_token_range + 1);
    
    for (int i = main_lower_token_range; i < main_upper_token_range; i++) {
        if (((token**)tokens->data)[i]->type == TYPE && ((token**)tokens->data)[i + 1]->type != OTHER) {
            fprintf(stderr, "ERROR: Line %i delceration after type is expected\n", ((token**)tokens->data)[i + 1]->pos_in_file);
        } else if (((token**)tokens->data)[i]->type == TYPE &&
                   ((token**)tokens->data)[i + 1]->type == OTHER &&
                   ((token**)tokens->data)[i + 2]->type == OPERATOR &&
                   strcmp(((token**)tokens->data)[i + 2]->data, "=") == 0) {
            // Assignment
            AST_node* assignment_node = malloc(sizeof(AST_node));
            init_AST_node(assignment_node);
            
            assignment_node->token = ((token**)tokens->data)[i + 1];
            assignment_node->node_type = VARIBLE;

            varible* varible = malloc(sizeof(varible));
            varible->type = get_type_from_str(types, ((token**)tokens->data)[i]->data);
            assignment_node->data = (void*)(varible);

            AST_node* value_node = malloc(sizeof(AST_node));
            init_AST_node(value_node);
            value_node = create_rhs_AST_node(types, tokens, i + 3);
            
            
            //printf("Varible of type %s and name %s was created with value %s\n", );
        }
    }
    //if (strcmp(((function*)main_node->data)->return_type->string, "void") != 0) {
    //    AST_node* main_return_node = malloc(sizeof(AST_node));
    //    //main_return_node->
                                                                                  //}
    
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
    dynamic_array tokens;
    da_init(&tokens, token*)
        
    while (pos < f.size) {
        token* t = get_next_token(&f, &pos);
        da_append(&tokens, t, token*);
    }

    // Generate default types
    dynamic_array ts;
    da_init(&ts, type);
    generate_default_types(&ts);
    
    clean_tokens(&tokens);
    for (int i = 0; i < tokens.count; i++) {
        ((token**)tokens.data)[i]->type = get_token_type(((token**)tokens.data)[i], &ts);
        printf("token at %i: %s and type %i\n", ((token**)tokens.data)[i]->pos_in_file, ((token**)tokens.data)[i]->data, ((token**)tokens.data)[i]->type);
    }






    // TYPE        = 0,
    // PARENTHESES = 1,
    // SEMICOLON   = 2,
    // NUMBER      = 3,
    // KEY_WORD    = 4,
    // OPERATOR    = 5,
    // OTHER       = 6,

    
    // Generate AST
    AST_node* main_node;
    generate_AST(main_node, &ts, &tokens);

    return 0;
}
