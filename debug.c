#include "minc.h"

void error(char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  fprintf(stderr, "error: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(1);
}

void dump_token(struct Token *p) {
  switch (p->type) {
  case TEOF:
    printf("TEOF\n");
    break;
  case TADD:
    printf("TADD\n");
    break;
  case TSUB:
    printf("TSUB\n");
    break;
  case TMUL:
    printf("TMUL\n");
    break;
  case TDIV:
    printf("TDIV\n");
    break;
  case TCLOSEBRACE:
    printf("TCLOSEBRACE\n");
    break;
  case TOPENBRACE:
    printf("TOPENBRACE\n");
    break;
  case TEQ:
    printf("TEQ\n");
    break;
  case TNEQ:
    printf("TNEQ\n");
    break;
  case TSEMICOLON:
    printf("TSEMICOLON\n");
    break;
  case TNUMBER:
    printf("TNUMBER, int_value=%d\n", p->int_value);
    break;
  }
}
