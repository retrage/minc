#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void read_term(void);
void read_mul_div(void);
void read_add_sub(void);

enum TokenType {
  TEOF,
  TNUMBER,
  TADD,
  TSUB,
  TMUL,
  TDIV,
  TOPENBRACE,
  TCLOSEBRACE,
};

struct Token {
  enum TokenType type;
  int int_value;
};

struct Token tokens[100];
struct Token *token = tokens - 1;

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
  return ++token;
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
    } else {
      if (num_idx > 0) {
        number[num_idx] = '\0';
        p->type = TNUMBER;
        p->int_value = atoi(number);
        p++;
        num_idx = 0;
      }
      switch (c) {
      case '+':
        p->type = TADD;
        break;
      case '-':
        p->type = TSUB;
        break;
      case '*':
        p->type = TMUL;
        break;
      case '/':
        p->type = TDIV;
        break;
      case '(':
        p->type = TOPENBRACE;
        break;
      case ')':
        p->type = TCLOSEBRACE;
        break;
      case EOF:
        p->type = TEOF;
        return;
      }
      p++;
    }
  }
}

#ifdef DEBUG
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
    case TNUMBER:
      printf("TNUMBER, int_value=%d\n", p->int_value);
      break;
  }
}
#endif

void read_term(void) {
  if ((token + 1)->type == TOPENBRACE) {
    token = get_token();
    read_add_sub();
    token = get_token();
    if (token->type != TCLOSEBRACE)
      error(") expected");
  } else if ((token + 1)->type == TNUMBER) {
    token = get_token();
    printf("\tmovl $%d, %%eax\n", token->int_value);
  } else {
#ifdef DEBUG
    printf("# read_term: Token ");
    dump_token((token + 1));
#endif
  }
}

void read_mul_div(void) {
  read_term();

#ifdef DEBUG
  printf("# read_mul_div: Token ");
  dump_token((token + 1));
#endif
  while ((token + 1)->type == TMUL || (token + 1)->type == TDIV) {
    token = get_token();
    enum TokenType op = token->type;
    printf("\tpushq %%rax\n");
    read_term();
    printf("\tmov %%rax, %%rcx\n");
    printf("\tpop %%rax\n");

    if (op == TMUL) {
      printf("\tmul %%ecx\n");
    } else if (op == TDIV) {
      printf("\tcqto\n");
      printf("\tdiv %%ecx\n");
    }
  }
}

void read_add_sub(void) {
  read_mul_div();

  struct Token *local_token;
  while ((token + 1)->type != TEOF) {
    printf("\tpushq %%rax\n");
    local_token = get_token();
#ifdef DEBUG
    printf("# read_add_sub: local_token ");
    dump_token(local_token);
#endif
    read_mul_div();
    printf("\tpop %%rdi\n");

    printf("\tmov %%rdi, %%rbx\n");
    if (local_token->type == TADD)
      printf("\taddl %%ebx, %%eax\n");
    else if (local_token->type == TSUB) {
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
