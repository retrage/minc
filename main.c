#include "minc.h"

static void init(void) {
  parse_init();
}

int main(int argc, char *argv[]) {
  init();
  tokenize();

  Vector *toplevels = parse_toplevel();

  for (int i = 0; i < vector_size(toplevels); i++)
    printf("# %s\n", node2s(vector_get(toplevels, i)));

  analyze_toplevel(toplevels);

  emit_toplevel(toplevels);

  return 0;
}
