#include <stdio.h>

int main(int argc, char *argv[]) {
  int val;

  scanf("%d", &val);

  printf(".globl main\n");
  printf("main:\n");
  printf("\tmovl $%d, %%eax\n", val);
  printf("\tret\n");

  return 0;
}
