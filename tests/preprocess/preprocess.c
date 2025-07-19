#  define ADD(a, b) (a                         \
                     + b)
#define NOT(a) (a)


int main() {
    int j = ADD(5, NOT(1 + 2));
    return j;
}
