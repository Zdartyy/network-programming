#include <stdio.h>
#include <stdbool.h>

bool is_printable_str(const char *str) {
    const unsigned char *p = (const unsigned char *)str;

    while (*p != '\0') {
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

    printf("dobry:     %s\n", is_printable_str(dobry) ? "true" : "false");
    printf("zly:       %s\n", is_printable_str(zly) ? "true" : "false");
    printf("graniczny: %s\n", is_printable_str(graniczny) ? "true" : "false");

    return 0;
}
