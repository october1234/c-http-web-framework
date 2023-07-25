#include <stdio.h>
#include <stdlib.h>

int main() {
    int *p = NULL;
    p = realloc(p, sizeof(int));
    p[0] = 100;
    printf("%d\n", p[0]);
}
