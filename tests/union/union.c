
typedef union u_t1 {
    char* a;
    int b;
    long c;
} u_t2;

int main() {
    u_t2 un;
    un.a = "test string";
    un.c = 3;
    un.b = 2;
    
    return un.b;
}
