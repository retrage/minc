/* From 8cc */

#include "minc.h"

Buffer *buffer_new(void) {
  Buffer *res = malloc(sizeof(Buffer));

  res->len = 0;
  res->buf_len = sizeof(void *) * 8;
  res->buf = malloc(res->buf_len);
  if (!res->buf)
    error("malloc failed");

  return res;
}

static void realloc_buffer(Buffer *b) {
  b->buf_len *= 2;
  b->buf = realloc(b->buf, b->buf_len);
  if (!b->buf)
    error("realloc failed");
}

void buffer_write(Buffer *b, char c) {
  if (b->buf_len == (b->len + 1))
    realloc_buffer(b);
  b->buf[b->len++] = c;
}

void buffer_append(Buffer *b, char *s, int len) {
  for (int i = 0; i < len; i++)
    buffer_write(b, s[i]);
}

char *buffer_body(Buffer *b) {
  return b->buf;
}

size_t buffer_size(Buffer *b) {
  return (size_t)b->len;
}

char *vformat(char *fmt, va_list ap) {
  Buffer *buf = buffer_new();
  va_list aq;
  while (1) {
    int avail = buf->buf_len - buf->len;
    va_copy(aq, ap);
    int written = vsnprintf(buf->buf + buf->len, avail, fmt, aq);
    va_end(aq);
    if (avail <= written) {
      realloc_buffer(buf);
      continue;
    }
    buf->len += written;
    return buffer_body(buf);
  }
}

char *format(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char *res = vformat(fmt, ap);
  va_end(ap);

  return res;
}
