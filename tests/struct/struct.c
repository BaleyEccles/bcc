
typedef struct {//  Stack pos
    char* a;  // 8 | 0
    int b;    // 2 | 8
    long c;   // 4 | 10 
} s_t;        // +
              // = 14

struct s2_t {
    int a[10];
};

int main() {
    int i = 1;
    s_t s;
    s.a = "test string";
    s.c = i;
    s.b = 3;
    s.b = 4;

    struct s2_t s2;
    s2.a[0] = 1;
    
    return s.b + s.c + i + s2.a[0];
}
