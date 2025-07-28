
typedef char* string;

typedef long ssize_t;

int sum_chars(string str, ssize_t str_size)
{
    int result = 0;
    for (int i = 0; i < str_size; i++) {
        result += (int)str[i];
    }

    return result;
}

int main()
{
    char* str = "abcdefg";
    ssize_t str_size = 7;
    
    int output = sum_chars((string)str, str_size);
    
    return output - 700;
}
