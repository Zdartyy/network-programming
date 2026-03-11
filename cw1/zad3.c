#include <stdio.h>
#include <stdbool.h>


bool is_printable_buf(const void *buf, int len) {
    const unsigned char *p   = (const unsigned char *)buf;
    const unsigned char *end = p + len;

    while (p < end) {
        if (*p < 32 || *p > 126) {
            return false;
        }
        p++;
    }

    return true;
}

int main(void) {
    char dobry[]  = "Hello, World!";
    char zly[]    = "Hello\x01World";  //0x01 (wartość 1) — niedrukowalny
    char graniczny[] = " ~";           // 32 i 126 — dokładnie na granicy

    printf("dobry:     %s\n", is_printable_buf(dobry,sizeof(dobry)-1) ? "true" : "false");
    printf("zly:       %s\n", is_printable_buf(zly,sizeof(zly)-1) ? "true" : "false");
    printf("graniczny: %s\n", is_printable_buf(graniczny, sizeof(graniczny)-1) ? "true" : "false");

    return 0;
}
