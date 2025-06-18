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

typedef struct {
    size_t count;
    char** names;
} types;

enum TOKENS {
    TYPE = 0,
    VARIBLE,
    FUNCTION,
    EXPRESSION,
    KEY_WORD,
};



void read_unitl_token(file* f, int* start_pos)
{
    types types;
    types.count = 2;
    char* t[2] =  {"int", "char"};
    types.names = t;

    
    int end_pos = 1;
    bool token_found = false;
    while (!token_found) {
        int len = end_pos - *start_pos;
        printf("len: %i\n", len);
        for (int i = 0; i < types.count; i++) {
            char* type = types.names[i];
            char current_str[100];
            strncpy(current_str, f->data + *start_pos, len);
            current_str[len] = (char)NULL;
            printf("comparing: \"%s\" and \"%s\", len: %i\n", type, current_str, len);
            if (strncmp(current_str, type, strlen(type)) == 0) {
                printf("type: %s found\n", type);
                token_found = true;
                i = types.count;
            }

        }
        end_pos++;
    }
}

int main()
{
    char name[] = "./tests/test1.c";
    file f;
    store_file(&f, name);

    int pos = 1;
    //while (true) {
    read_unitl_token(&f, &pos);
    pos++;
        
    //}
    
    // 

    

    return 0;
}
