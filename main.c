#include "minc.h"

static void init(void) {
  token = tokens - 1;
  ident = map_new();
  rbp_offset = 0x0;
}

int main(int argc, char *argv[]) {
  init();
  tokenize();
  printf(".globl main\n");
  parse();

  return 0;
}
