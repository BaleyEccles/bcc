#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "tokenizer.h"

// #define MACRO(a, b, c) (a + b + c)
//         ^name  ^inputs   ^ outputs
typedef struct {
    token* name;
    dynamic_array* output;
    dynamic_array* inputs;
} define;


void preprocess_file(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths);

#endif // PREPROCESSOR_H
