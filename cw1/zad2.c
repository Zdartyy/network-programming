#include <stdio.h>

void drukuj_alt(int *tablica, int liczba_elementow){
    int *p = tablica;
    int *end = tablica + liczba_elementow;

    while (p < end) {
        if (*p > 10 && *p <100) {
            printf("drukuje: %d\n", *p);
        }
        p++;
    }
}

int main() {
    int liczby[5];
    int n = 0;

    for (int i = 0; i < 5; i++) {
        scanf("%d", &liczby[i]);

        if (liczby[i] == 0) {
            break;
        }

        n++;
    }

    drukuj_alt(liczby, n);

    return 0;
}
