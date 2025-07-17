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

#define da_allocate(DA, DA_TYPE, SIZE)                                  \
    {                                                                   \
        DA_TYPE* new_data = malloc(((SIZE)) * sizeof(DA_TYPE));         \
        memcpy(new_data, (DA)->data, (DA)->size * sizeof(DA_TYPE));     \
        free((DA)->data);                                               \
        (DA)->data = new_data;                                          \
        (DA)->size = (SIZE);                                            \
    }

// Shfit the dynamic array right at LOC by LEN
#define da_shift(DA, DA_TYPE, LOC, LEN)                                 \
    {                                                                   \
        if ((DA)->count + (LEN) >= (DA)->size) {                        \
            da_allocate(DA, DA_TYPE, 2*(((DA)->count + (LEN))));        \
        }                                                               \
        memmove((DA)->data + sizeof(DA_TYPE)*((LOC) + (LEN)), (DA)->data + sizeof(DA_TYPE)*(LOC), ((DA)->count*sizeof(DA_TYPE) - (LOC))*sizeof(DA_TYPE)); \
        (DA)->count += (LEN);                                           \
    }

