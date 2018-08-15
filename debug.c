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

char *dump_token(Token *p) {
  switch (p->type) {
  case TEOF:
    return "TEOF";
  case TADD:
    return "TADD";
  case TSUB:
    return "TSUB";
  case TMUL:
    return "TMUL";
  case TDIV:
    return "TDIV";
  case TCLOSEBRACE:
    return "TCLOSEBRACE";
  case TOPENBRACE:
    return "TOPENBRACE";
  case TEQ:
    return "TEQ";
  case TNEQ:
    return "TNEQ";
  case TSEMICOLON:
    return "TSEMICOLON";
  case TIDENTIFIER:
    return "TIDENTIFIER";
  case TASSIGN:
    return "TASSIGN";
  case TTESTVECTOR:
    return "TTESTVECTOR";
  case TTESTMAP:
    return "TTESTMAP";
  case TNUMBER:
    return "TNUMBER";
  }
  return "UNK";
}
