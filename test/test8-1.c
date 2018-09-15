#include <stdio.h>
#include <stdlib.h>

void alloc4(int **p, int a1, int a2, int a3, int a4) {
  *p = malloc(sizeof(int) * 4);
  (*p)[0] = a1;
  (*p)[1] = a2;
  (*p)[2] = a3;
  (*p)[3] = a4;
}

void print_int(int a) {
  printf("%d\n", a);
}
