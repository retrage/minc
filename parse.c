#include "minc.h"

Token *get_token(void) {
  return ++token;
}

void tokenize(void) {
  char c;
  Token *p = tokens;
  char number[128];
  int num_idx = 0;
  char string[128];
  int str_idx = 0;

  while (1) {
    c = getchar();
    if (c >= '0' && c <= '9') {
      number[num_idx] = c;
      num_idx++;
    } else if (c >= 'A' && c <= 'z') {
      string[str_idx] = c;
      str_idx++;
    } else {
      if (num_idx > 0) {
        number[num_idx] = '\0';
        p->type = TNUMBER;
        p->int_value = atoi(number);
        p++;
        num_idx = 0;
      }
      if (str_idx > 0) {
        string[str_idx] = '\0';
        if (!strcmp(string, "test_vector"))
          p->type = TTESTVECTOR;
        else if (!strcmp(string, "test_map"))
          p->type = TTESTMAP;
        else
          error("test_vector expected");
        p++;
        str_idx = 0;
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
      case ' ':
      case '\t':
        continue;
      }
      p++;
    }
  }
}

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
    TokenType op = token->type;
    printf("\tpushq %%rax\n");
    read_term();
    printf("\tmov %%rax, %%rdi\n");
    printf("\tpop %%rax\n");

    if (op == TMUL) {
      printf("\tmul %%edi\n");
    } else if (op == TDIV) {
      printf("\tcqto\n");
      printf("\tdiv %%edi\n");
    }
  }
}

void read_add_sub(void) {
  read_mul_div();

  while ((token + 1)->type == TADD || (token + 1)->type == TSUB) {
    token = get_token();
    TokenType op = token->type;
    printf("\tpushq %%rax\n");
    read_mul_div();
    printf("\tmov %%rax, %%rdi\n");
    printf("\tpop %%rax\n");

    if (op == TADD)
      printf("\taddl %%edi, %%eax\n");
    else if (op == TSUB) {
      printf("\tsubl %%edi, %%eax\n");
    }
  }
}

void read_eq_neq(void) {
  read_add_sub();

  while ((token + 1)->type == TEQ || (token + 1)->type == TNEQ) {
    token = get_token();
    TokenType op = token->type;
    printf("\tpushq %%rax\n");
    read_add_sub();
    printf("\tmov %%rax, %%rdi\n");
    printf("\tpop %%rax\n");

    printf("\tcmpl %%eax, %%edi\n");
    if (op == TEQ) {
      printf("\tsete %%al\n");
    } else if (op == TNEQ) {
      printf("\tsetne %%al\n");
    }
    printf("\tmovzbl %%al, %%eax\n");
  }
}

void read_expr(void) {
  read_eq_neq();

  while ((token + 1)->type == TSEMICOLON) {
    token = get_token();
    read_eq_neq();
  }
}

void parse(void) {
  if ((token + 1)->type == TTESTVECTOR) {
    int res = test_vector();
    printf("\tmovl $%d, %%eax\n", res);
  } else if ((token + 1)->type == TTESTMAP) {
    int res = test_map();
    printf("\tmovl $%d, %%eax\n", res);
  } else {
    while ((token + 1)->type != TEOF)
      read_expr();
  }
  printf("\tret\n");
}
