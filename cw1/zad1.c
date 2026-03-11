#include <stdio.h>

void drukuj(int tablica[], int liczba_elementow) {
    for (int i = 0; i < liczba_elementow; i++) {
        if (tablica[i] > 10 && tablica[i] < 100) {
            printf("drukuje: %d\n", tablica[i]);
        }
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

    drukuj(liczby, n);

    return 0;
}
