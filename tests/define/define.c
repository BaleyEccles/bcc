#include <stdio.h>

#define TEST 1

#undef TEST
#define TEST 5

int main() {
    return TEST;
}
