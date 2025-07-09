
int sum_chars(char* str, int str_size)
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
    int str_size = 7;
    
    int output = sum_chars(str, str_size);
    
    return output - 700;
}
