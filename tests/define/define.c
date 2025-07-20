#define TEST 1

#undef TEST
#define TEST 5

#ifndef NOT_DEFINED

#ifdef NOT_DEFINED
#define TEST2 5
#else
#define TEST2 2
#endif

#endif

int main() {
    return TEST + TEST2;
}
