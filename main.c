#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

enum TokenType {
  TEOF,
  TNUMBER,
  TPLUS,
  TMINUS,
  TMUL,
  TDIV,
};

struct Token {
  enum TokenType type;
  int int_value;
};

struct Token tokens[100];
struct Token *token;

void error(char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  fprintf(stderr, "error: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(1);
}

struct Token *get_token(void) {
  static struct Token *p = tokens;
  return p++;
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
    } else if (c == '*') {
      if (num_idx > 0) {
        number[num_idx] = '\0';
        p->type = TNUMBER;
        p->int_value = atoi(number);
        p++;
        num_idx = 0;
      }
      p->type = TMUL;
      p++;
    } else if (c == '/') {
      if (num_idx > 0) {
        number[num_idx] = '\0';
        p->type = TNUMBER;
        p->int_value = atoi(number);
        p++;
        num_idx = 0;
      }
      p->type = TDIV;
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

void read_mul_div(void) {
  token = get_token();
  if (token->type != TNUMBER)
    return;
  else
    printf("\tmovl $%d, %%eax\n", token->int_value);

  while ((token+1)->type == TMUL || (token+1)->type == TDIV) {
    token = get_token();
    enum TokenType op = token->type;
    token = get_token();
    if (token->type != TNUMBER)
      error("number expected");
    if (op == TMUL) {
      printf("\tmovl $%d, %%ecx\n", token->int_value);
      printf("\tmul %%ecx\n");
    } else if (op == TDIV) {
      printf("\tmovl $%d, %%ecx\n", token->int_value);
      printf("\tcqto\n");
      printf("\tdiv %%ecx\n");
    }
  }
}

void read_add_sub(void) {
  read_mul_div();

  struct Token *local_token;
  while ((token+1)->type != TEOF) {
    printf("\tpushq %%rax\n");
    local_token = get_token();
    read_mul_div();
    printf("\tpop %%rdi\n");

    printf("\tmov %%rdi, %%rbx\n");
    if (local_token->type == TPLUS)
      printf("\taddl %%ebx, %%eax\n");
    else if (local_token->type == TMINUS) {
      printf("\tsubl %%eax, %%ebx\n");
      printf("\tmov %%ebx, %%eax\n");
    } else
      error("+ or - exptected");
  }
}

void parse(void) {
  read_add_sub();
  printf("\tret\n");
}

int main(int argc, char *argv[]) {
  tokenize();
  printf(".globl main\n");
  printf("main:\n");
  parse();

  return 0;
}
