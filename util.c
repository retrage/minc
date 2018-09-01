#include "minc.h"

int calc_offset(Map *env) {
  int offset = 0;
  Vector *keys = map_keys(env);
  for (int i = 0; i < vector_size(keys); i++) {
    Type *ty = map_get(env, vector_get(keys, i));
    offset += ty->size;
  }

  return offset;
}

