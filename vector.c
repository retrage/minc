#include "minc.h"

Vector *vector_new(void) {
  Vector *res = malloc(sizeof(Vector));
  if (!res)
    error("failed to malloc");

  res->vec_len = 0;
  res->buf_len = sizeof(void *) * 128;
  res->buf = malloc(res->buf_len);
  if (!res->buf)
    error("failed to malloc");

  return res;
}

void vector_push(Vector *vec, void *item) {
  if (vec->vec_len > vec->buf_len / sizeof(void *)) {
    vec->buf = realloc(vec->buf, vec->buf_len * 2);
    if (!vec->buf)
      error("failed to realloc");
    vec->buf_len = vec->buf_len * 2;
  }

  vec->buf[vec->vec_len] = item;
  vec->vec_len++;
}

void *vector_get(Vector *vec, size_t idx) {
  if (idx > vec->vec_len)
    error("idx is too big");

  return vec->buf[idx];
}

size_t vector_size(Vector *vec) {
  return (size_t)vec->vec_len;
}
