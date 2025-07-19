#  define ADD(a, b) (a                         \
                     + b)
#define NOT(a) (a)
#define TEST 1

int main() {
    int j = ADD(5, NOT(TEST + 2));
    return j;
}
