#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "tokenizer.h"

void preprocess_file(dynamic_array* tokens, dynamic_array* defines, dynamic_array* include_paths);

#endif // PREPROCESSOR_H
