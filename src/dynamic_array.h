typedef struct {                            
    void* data;
    int count;
    int size;
} dynamic_array;
                                         

#define da_append(DA, INPUT_DATA, DA_TYPE)                              \
    {                                                                   \
        if ((DA)->count >= (DA)->size) {                                \
            DA_TYPE* new_data = malloc(2 * (DA)->size * sizeof(DA_TYPE)); \
            memcpy(new_data, (DA)->data, (DA)->size * sizeof(DA_TYPE)); \
            free((DA)->data);                                           \
            (DA)->data = new_data;                                      \
            (DA)->size = 2 * (DA)->size;                                \
        }                                                               \
        ((DA_TYPE*)(DA)->data)[(DA)->count] = (INPUT_DATA);             \
        (DA)->count++;                                                  \
    }                                                                   

#define da_init(DA, DA_TYPE)                                        \
    {                                                               \
        (DA)->size = 256;                                           \
        DA_TYPE* new_data = malloc((DA)->size * sizeof(DA_TYPE));   \
        (DA)->data = new_data;                                      \
        (DA)->count = 0;                                            \
    }

