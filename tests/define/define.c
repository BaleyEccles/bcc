#define TEST 1

#undef TEST
#define TEST 5

#ifndef NOT_DEFINED

#ifdef TEST
#define TEST2 1
#endif

#endif

int main() {
    return TEST + TEST2;
}
