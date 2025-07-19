#  define ADD(a, b) (a                         \
                     + b)
#define NOT(a) (a)
#define TEST 1
#define TEST2 1

#if (TEST - 1) && \
    defined TEST2 && defined(TEST2)
#define VAL 2
#elif TEST2
#define VAL 3
#endif

int main() {
    int j = ADD(5, NOT(VAL + 2));
    return j;
}
