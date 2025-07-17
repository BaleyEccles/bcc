#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "tokenizer.h"

void preprocess_file(dynamic_array* tokens);

#endif // PREPROCESSOR_H
