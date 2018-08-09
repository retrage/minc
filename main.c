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
  TEQ,
  TNEQ,
  TSEMICOLON,
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
      case '=':
        c = getchar();
        if (c == '=')
          p->type = TEQ;
        else
          error("== expected");
        break;
      case '!':
        c = getchar();
        if (c == '=')
          p->type = TNEQ;
        else
          error("!= expected");
        break;
      case ';':
        p->type = TSEMICOLON;
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
  }
}

void read_mul_div(void) {
  read_term();

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

  while ((token + 1)->type == TADD || (token + 1)->type == TSUB) {
    token = get_token();
    enum TokenType op = token->type;
    printf("\tpushq %%rax\n");
    read_mul_div();
    printf("\tpop %%rdi\n");

    printf("\tmov %%rdi, %%rbx\n");
    if (op == TADD)
      printf("\taddl %%ebx, %%eax\n");
    else if (op == TSUB) {
      printf("\tsubl %%eax, %%ebx\n");
      printf("\tmov %%ebx, %%eax\n");
    }
  }
}

void read_eq_neq(void) {
  read_add_sub();

  while ((token + 1)->type == TEQ || (token + 1)->type == TNEQ) {
    token = get_token();
    enum TokenType op = token->type;
    printf("\tpushq %%rax\n");
    read_add_sub();
    printf("\tpop %%rdi\n");

    printf("\tmov %%rdi, %%rbx\n");
    if (op == TEQ) {
      printf("\tcmpl %%eax, %%ebx\n");
      printf("\tsete %%al\n");
      printf("\tmovzbl %%al, %%eax\n");
    } else if (op == TNEQ) {
      printf("\tcmpl %%eax, %%ebx\n");
      printf("\tsetne %%al\n");
      printf("\tmovzbl %%al, %%eax\n");
    }
  }
}

void read_expr(void) {
  read_eq_neq();

  while ((token + 1)->type == TSEMICOLON) {
    token = get_token();
    enum TokenType op = token->type;
    printf("\tpushq %%rax\n");
    read_eq_neq();
    printf("\tpop %%rdi\n");

    printf("\tmov %%rdi, %%rbx\n");
    if (op == TSEMICOLON) {
      /* do nothing */
    }
  }
}

void parse(void) {
  while ((token + 1)->type != TEOF)
    read_expr();
  printf("\tret\n");
}

int main(int argc, char *argv[]) {
  tokenize();
  printf(".globl main\n");
  printf("main:\n");
  parse();

  return 0;
}
