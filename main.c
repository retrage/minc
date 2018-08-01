#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

enum TokenType {
  TNUMBER,
  TPLUS,
  TMINUS,
  TEOF,
};

struct Token {
  enum TokenType type;
  int int_value;
};

struct Token tokens[100];

void error(char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  fprintf(stderr, "error: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(1);
}

void tokenize(void) {
  char c;
  struct Token *p = tokens;
  char number[128];
  int num_idx = 0;

  while (1) {
    c = getchar();
    if (c >= '0' && c <= '9') {
      number[num_idx] = c;
      num_idx++;
    } else if (c == '+') {
      if (num_idx > 0) {
        number[num_idx] = '\0';
        p->type = TNUMBER;
        p->int_value = atoi(number);
        p++;
        num_idx = 0;
      }
      p->type = TPLUS;
      p++;
    } else if (c == '-') {
      if (num_idx > 0) {
        number[num_idx] = '\0';
        p->type = TNUMBER;
        p->int_value = atoi(number);
        p++;
        num_idx = 0;
      }
      p->type = TMINUS;
      p++;
    } else if (c == EOF) {
      if (num_idx > 0) {
        number[num_idx] = '\0';
        p->type = TNUMBER;
        p->int_value = atoi(number);
        p++;
        num_idx = 0;
      }
      p->type = TEOF;
      break;
    }
  }
}

struct Token *get_token(void) {
  static struct Token *p = tokens;
  return p++;
}

void parse(void) {
  struct Token *token = get_token();
  if (token->type != TNUMBER)
    error("number expected");
  else
    printf("\tmovl $%d, %%eax\n", token->int_value);

  while (token != NULL) {
    token = get_token();
    if (token->type == TEOF) {
      printf("\tret\n");
      return;
    }

    enum TokenType op;
    if (token->type != TPLUS && token->type != TMINUS)
      error("+ or - expected");
    else
      op = token->type;

    token = get_token();
    if (token->type != TNUMBER)
      error("number expected");
    else {
      if (op == TPLUS)
        printf("\taddl $%d, %%eax\n", token->int_value);
      else if (op == TMINUS)
        printf("\tsubl $%d, %%eax\n", token->int_value);
      else
        error("unknown op");
    }
  }
}

int main(int argc, char *argv[]) {
  tokenize();
  printf(".globl main\n");
  printf("main:\n");
  parse();

  return 0;
}
