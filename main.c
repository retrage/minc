#include "minc.h"

static void init(void) {
  parse_init();
}

int main(int argc, char *argv[]) {
  init();
  tokenize();

  Vector *toplevels = parse_toplevel();

  analyze_toplevel(toplevels);

  for (int i = 0; i < vector_size(toplevels); i++)
    printf("# %s\n", node2s(vector_get(toplevels, i)));

  emit_toplevel(toplevels);

  return 0;
}
