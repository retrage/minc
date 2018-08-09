#include "minc.h"

static int map_get_index(Map *map, char *key) {
  for (int idx = vector_size(map->keys) - 1; 0 <= idx; idx--) {
    if (!strcmp((char *)vector_get(map->keys, idx), key))
      return idx;
  }

  return -1;
}

Map *map_new(void) {
  Map *res = malloc(sizeof(Map));
  if (!res)
    error("failed to malloc");

  res->map_len = 0;
  res->keys = vector_new();
  res->values = vector_new();

  return res;
}

void map_push(Map *map, char *key, void *value) {
  vector_push(map->keys, key);
  vector_push(map->values, value);
  map->map_len++;
}

void *map_get(Map *map, char *key) {
  int idx = map_get_index(map, key);
  if (idx == -1)
    return NULL;

  return vector_get(map->values, idx);
}

size_t map_size(Map *map) {
  size_t keys_size = vector_size(map->keys);
  if (keys_size < 2)
    return keys_size;

  int dup_count = 0;
  for (int rhs = 0; rhs < keys_size; rhs++) {
    for (int lhs = rhs+1; lhs < keys_size; lhs++) {
      if (!strcmp(vector_get(map->keys, rhs), vector_get(map->keys, lhs)))
        dup_count++;
    }
  }

  return keys_size - dup_count;
}
