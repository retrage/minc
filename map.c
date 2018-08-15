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
  Vector *keys = map_keys(map);

  return vector_size(keys);
}

Vector *map_keys(Map *map) {
  Vector *keys = vector_new();
  size_t keys_size = vector_size(map->keys);
  if (keys_size == 0)
    return keys;

  vector_push(keys, vector_get(map->keys, 0));
  for (int rhs = 0; rhs < keys_size; rhs++) {
    char *rhs_string = vector_get(map->keys, rhs);
    int is_dup = 0;
    for (int lhs = 0; lhs < vector_size(keys); lhs++) {
      char *lhs_string = vector_get(keys, lhs);
      if (!strcmp(rhs_string, lhs_string))
        is_dup++;
    }
    if (!is_dup)
      vector_push(keys, rhs_string);
  }

  return keys;
}
