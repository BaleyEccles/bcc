
typedef struct {//  Stack pos
    char* a;  // 8 | 0
    int b;    // 2 | 8
    long c;   // 4 | 10 
} s_t;        // +
              // = 14

int main() {
    int i = 1;
    s_t s;
    s.a = "test string";
    s.c = 7;
    s.b = 3;
    s.b = 4;
    
    return s.b + s.c - i;
}
