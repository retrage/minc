#include "minc.h"

static void init(void) {
  token = tokens - 1;
  ident = map_new();
  rbp_offset = 0x0;
}

static void func_prologue(void) {
  printf("\tpush %%rbp\n");
  printf("\tmov %%rsp, %%rbp\n");

  Vector *ident_keys = map_keys(ident);
  for (int i = 0; i < vector_size(ident_keys); i++) {
    map_push(ident, vector_get(ident_keys, i), (void *)(rbp_offset - 8));
    printf("\tmovl $8, %%edi\n");
    printf("\tsub %%rdi, %%rsp\n");
    rbp_offset -= 8;
  }
}

static void func_epilogue(void) {
  for (int i = 0; i < map_size(ident); i++) {
    rbp_offset += 8;
    printf("\tmovl $8, %%edi\n");
    printf("\tadd %%rdi, %%rsp\n");
  }

  if (map_size(ident) > 0)
    printf("\txor %%rax, %%rax\n");

  printf("\tpop %%rbp\n");
}

int main(int argc, char *argv[]) {
  init();
  tokenize();
  printf(".globl main\n");
  printf("main:\n");
  func_prologue();
  parse();
  func_epilogue();
  printf("\tret\n");

  return 0;
}
