
int main()
{
    char* str = "abcdefg";
    int size = 0;
    for (int i = 0; i < 10; i++) {
        size += i;
    }
    int array1[size];
    int array2[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};

    array1[2] = 5;
    return array1[2] + array2[4];
}
