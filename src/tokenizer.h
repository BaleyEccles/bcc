#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "dynamic_array.h"


#define ALLOC_STR(TOKEN, STR)                   \
    (TOKEN)->data = malloc(strlen(STR) + 1);    \
    if ((TOKEN)->data != NULL) {                \
        strcpy(TOKEN->data, STR);               \
    }

typedef enum {
    ERROR = 0,
    TYPE,
    PARENTHESES,
    SEMICOLON,
    NUMBER,
    OTHER,
    ACCESS_MEMBER,
    PAREN_OPEN,     //Parentheses
    PAREN_CLOSE,
    PAREN_CURLY_OPEN,
    PAREN_CURLY_CLOSE,
    PAREN_SQUARE_OPEN,
    PAREN_SQUARE_CLOSE,
    AUTO,     // Key Words
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
    POST_INCREMENT,    // Operations
    POST_DECREMENT,
    UNARY_PLUS,
    UNARY_MINUS,
    LOGICAL_NOT,
    BITWISE_NOT,
    TIMES,
    DIVIDE,
    MODULO,
    PLUS,
    MINUS,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    LESS_THAN,
    LESS_THAN_OR_EQUALS,
    GREATER_THAN,
    GREATER_THAN_OR_EQUALS,
    LOGICAL_EQUALS,
    NOT_EQUALS,
    BITWISE_AND,
    BITWISE_XOR,
    BITWISE_OR,
    LOGICAL_AND,
    LOGICAL_OR,
    TERNARY_CONDITIONAL,
    COLON,
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
} TOKEN_TYPE;


typedef struct {
    char* string;
    int size; // In bytes
    int ptr_count;
    TOKEN_TYPE type_type;
    void* data;
} type;

typedef struct {
    dynamic_array* members;
} union_data;

typedef struct {
    dynamic_array* members;
} struct_data;

typedef struct {
    type* t;
} type_data;


typedef struct {
    TOKEN_TYPE type;
    int pos_in_file;
    char* data;
} token;

typedef struct {
    char* string;
    TOKEN_TYPE type;
} mapping;

int find_comma(dynamic_array* tokens, int start, int end);
int find_semi_colon(dynamic_array* tokens, int start_location);
int find_semi_colon_skip_parentheses(dynamic_array* tokens, int start_location);
int get_token_location(dynamic_array* tokens, token* t);
type* get_type_from_str(dynamic_array* types, char* str);
bool token_is_type(token* t, dynamic_array* ts);
type* get_type(dynamic_array* tokens, dynamic_array* types, token* t);
bool token_is_parentheses(token* t);
TOKEN_TYPE get_token_type_parentheses(token* t);
int get_closing_paren_location(dynamic_array* tokens, int starting_token_location);
bool token_is_modifier(token* t);
bool token_is_string(token* t);
bool token_is_number(token* t);
TOKEN_TYPE get_token_type_key_word(token* t);
bool token_is_key_word(token* t);
bool token_is_operator(token* t);
TOKEN_TYPE get_token_type_operator(token* t);
bool token_is_semicolon(token* t);
void get_token_type(dynamic_array* ts, dynamic_array* tokens, token* t);
bool is_token_end(dynamic_array* tokens, char* str, char c_next);
bool is_token(FILE* f, dynamic_array* tokens, char* current_str, int* start_pos, int end_pos);
token* get_next_token(FILE* f, dynamic_array* tokens, int* start_pos);
void remove_bad_chars(char* data);
void clean_tokens(dynamic_array* tokens);
void remove_comments(dynamic_array* tokens);
void untabbify_tokens(dynamic_array* tokens);

type* get_number_type(dynamic_array* types, token* t);
type* get_string_type(dynamic_array* types);

bool type_is_string(type* ty);

type* get_type_from_name_and_ptr_count(dynamic_array* types, char* str, int ptr_count);
type* get_dereferenced_type(dynamic_array* types, type* t);

void generate_type(dynamic_array* ts, dynamic_array* tokens, token* t);


// https://en.cppreference.com/w/cpp/language/operator_precedence.html
// This is also in reverse order of precedence
static const mapping operator_mapping[] = {
    {"++", POST_INCREMENT},
    {"--", POST_DECREMENT},
    {"+", PLUS},
    {"-", MINUS},
    {"!", LOGICAL_NOT},
    {"~", BITWISE_NOT},

    {".", ACCESS_MEMBER},
    {"->", ACCESS_MEMBER},
    
    {"*", TIMES},
    {"/", DIVIDE},
    {"%", MODULO},

    {"+", PLUS},
    {"-", MINUS},

    {"<<", LEFT_SHIFT},
    {">>", RIGHT_SHIFT},

    {"<", LESS_THAN},
    {"<=", LESS_THAN_OR_EQUALS},
    {">", GREATER_THAN},
    {">=", GREATER_THAN_OR_EQUALS},

    {"==", LOGICAL_EQUALS},
    {"!=", NOT_EQUALS},

    {"&", BITWISE_AND},

    {"^", BITWISE_XOR},

    {"|", BITWISE_OR},

    {"&&", LOGICAL_AND},

    {"||", LOGICAL_OR},

    {"?", TERNARY_CONDITIONAL},
    {":", COLON},

    {"=", EQUALS},
    {"+=", PLUS_EQUALS},
    {"-=", MINUS_EQUALS},
    {"*=", TIMES_EQUALS},
    {"/=", DIVIDE_EQUALS},
    {"%=", MODULO_EQUALS},
    {"&=", AND_EQUALS},
    {"|=", OR_EQUALS},
    {"^=", XOR_EQUALS},
    {"<<=", LEFT_SHIFT_EQUALS},
    {">>=", RIGHT_SHIFT_EQUALS},
};


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


static const mapping parentheses_mapping[] = {
    {"(", PAREN_OPEN},
    {")", PAREN_CLOSE},
    {"{", PAREN_CURLY_OPEN},
    {"}", PAREN_CURLY_CLOSE},
    {"[", PAREN_SQUARE_OPEN},
    {"]", PAREN_SQUARE_CLOSE},
};

#endif // TOKENIZER_H
