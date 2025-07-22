#  define ADD(a, b) (a                         \
                     + b)
#define NOT(a) (a)
#define TEST 0
#define TEST3 1

#if !TEST &&                               \
    !defined TEST2 || defined(TEST3)
#define VAL 2
#elif TEST2
#define VAL 3
#endif

int main() {
    int j = ADD(5, NOT(VAL + 2));
    return j;
}
