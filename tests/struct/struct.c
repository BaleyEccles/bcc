
typedef struct {
    char* a;
    int b;
    long c;
} s_t;

int main() {
    s_t s;
    s.a = "test string";
    s.c = 3;
    s.b = 2;
    
    return s.b + s.c;
}
