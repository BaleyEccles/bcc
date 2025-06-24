#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "dynamic_array.h"

typedef struct {
    int size;
    char* data;
} file;

typedef struct {
    char* string;
} type;

typedef enum {
    ERROR = 0,
    TYPE,
    PARENTHESES,
    SEMICOLON,
    NUMBER,
    OTHER,
     
    //Parentheses
    PAREN_OPEN,
    PAREN_CLOSE,
    PAREN_CURLY_OPEN,
    PAREN_CURLY_CLOSE,
    PAREN_SQUARE_OPEN,
    PAREN_SQUARE_CLOSE,

    // Key Words
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

    // Operations
    POST_INCREMENT,
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
    
    BOOL_LESS_THAN,
    BOOL_LESS_THAN_OR_EQUALS,
    BOOL_GREATER_THAN,
    BOOL_GREATER_THAN_OR_EQUALS,
    
    BOOL_EQUALS,
    BOOL_NOT_EQUALS,
    
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
    TOKEN_TYPE type;
    int pos_in_file;
    char* data;
} token;

typedef struct {
    char* string;
    TOKEN_TYPE type;
} mapping;

int find_semi_colon(dynamic_array* tokens, int start_location);
int get_token_location(dynamic_array* tokens, token* t);
type* get_type_from_str(dynamic_array* types, char* str);
void store_file(file* f, char* file_name);
bool token_is_type(token* t, dynamic_array ts);
bool token_is_parentheses(token* t);
TOKEN_TYPE get_token_type_parentheses(token* t);
bool token_is_number(token* t);
TOKEN_TYPE get_token_type_key_word(token* t);
bool token_is_key_word(token* t);
bool token_is_operator(token* t);
TOKEN_TYPE get_token_type_operator(token* t);
bool token_is_semicolon(token* t);
TOKEN_TYPE get_token_type(token* t, dynamic_array* ts);
bool is_token_end(char c);
bool is_token(file* f, char* current_str, int* start_pos, int end_pos);
token* get_next_token(file* f, int* start_pos);
void remove_bad_chars(char* data);
void clean_tokens(dynamic_array* tokens);


// https://en.cppreference.com/w/cpp/language/operator_precedence.html
// This is also in reverse order of precedence
static const mapping operator_mapping[] = {
    {"++", POST_INCREMENT},
    {"--", POST_DECREMENT},
    {"+", PLUS},
    {"-", MINUS},
    {"!", LOGICAL_NOT},
    {"~", BITWISE_NOT},

    {"*", TIMES},
    {"/", DIVIDE},
    {"%", MODULO},

    {"+", PLUS},
    {"-", MINUS},

    {"<<", LEFT_SHIFT},
    {">>", RIGHT_SHIFT},

    {"<", BOOL_LESS_THAN},
    {"<=", BOOL_LESS_THAN_OR_EQUALS},
    {">", BOOL_GREATER_THAN},
    {">=", BOOL_GREATER_THAN_OR_EQUALS},

    {"==", BOOL_EQUALS},
    {"!=", BOOL_NOT_EQUALS},

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


#endif // TOKENIZER_H
