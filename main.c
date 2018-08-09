#include "minc.h"

void init(void) {
  token = tokens - 1;
}

int main(int argc, char *argv[]) {
  init();
  tokenize();
  printf(".globl main\n");
  printf("main:\n");
  parse();

  return 0;
}
