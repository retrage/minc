#include "minc.h"

int test_vector(void) {
  Vector *vec = vector_new();
  if (vector_size(vec) != 0)
    return 1;

  char *string1 = malloc(sizeof(char)*128);
  strcpy(string1, "deadbeef");
  vector_push(vec, string1);
  if (vector_size(vec) != 1)
    return 2;
  if (strcmp((char *)vector_get(vec, 0), "deadbeef"))
    return 3;

  char *string2 = malloc(sizeof(char)*128);
  strcpy(string2, "the quick brown fox jumps over the lazy dog");
  vector_push(vec, string2);
  if (vector_size(vec) != 2)
    return 4;
  if (strcmp((char *)vector_get(vec, 1), "the quick brown fox jumps over the lazy dog"))
    return 5;

  return 0;
}
