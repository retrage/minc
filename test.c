#include "minc.h"

int test_vector(void) {
  Vector *vec = vector_new();
  if (vector_size(vec) != 0)
    return 1;

  char *string1 = malloc(sizeof(char) * 128);
  strcpy(string1, "deadbeef");
  vector_push(vec, string1);
  if (vector_size(vec) != 1)
    return 2;
  if (strcmp((char *)vector_get(vec, 0), "deadbeef"))
    return 3;

  char *string2 = malloc(sizeof(char) * 128);
  strcpy(string2, "the quick brown fox jumps over the lazy dog");
  vector_push(vec, string2);
  if (vector_size(vec) != 2)
    return 4;
  if (strcmp((char *)vector_get(vec, 1),
             "the quick brown fox jumps over the lazy dog"))
    return 5;

  return 0;
}

int test_map(void) {
  Map *map = map_new();
  if (!map)
    return 1;
  if (map_size(map) != 0)
    return 2;

  map_push(map, "foo", (void *)1);
  if (map_get(map, "foo") != (void *)1)
    return 3;
  if (map_size(map) != 1)
    return 4;

  map_push(map, "bar", (void *)2);
  map_push(map, "baz", (void *)3);
  if (map_get(map, "bar") != (void *)2)
    return 5;
  if (map_get(map, "baz") != (void *)3)
    return 6;
  if (map_size(map) != 3)
    return 7;

  map_push(map, "bar", (void *)4);
  if (map_get(map, "bar") != (void *)4)
    return 8;
  if (map_size(map) != 3)
    return 9;

  Vector *keys = map_keys(map);
  if (vector_size(keys) != 3)
    return vector_size(keys) + 100;

  if (strcmp((char *)vector_get(keys, 0), "foo"))
    return 11;
  if (strcmp((char *)vector_get(keys, 1), "bar"))
    return 12;
  if (strcmp((char *)vector_get(keys, 2), "baz"))
    return 13;

  return 0;
}
