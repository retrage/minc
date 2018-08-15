#include "minc.h"

static void func_prologue(void);
static void func_epilogue(void);

char *registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

Token *get_token(void) { return ++token; }

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
    } else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_') {
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
        else if (!strcmp(string, "return"))
          p->type = TRETURN;
        else {
          p->type = TIDENTIFIER;
          strcpy(p->identifier, string);
          map_push(ident, p->identifier, (void *)0);
        }
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
        p->type = TOPENPARENTHESIS;
        break;
      case ')':
        p->type = TCLOSEPARENTHESIS;
        break;
      case '=':
        c = getchar();
        if (c == '=')
          p->type = TEQ;
        else
          p->type = TASSIGN;
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
      case ',':
        p->type = TCOMMA;
        break;
      case '{':
        p->type = TOPENBRACE;
        break;
      case '}':
        p->type = TCLOSEBRACE;
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
  if ((token + 1)->type == TOPENPARENTHESIS) {
    token = get_token();
    read_add_sub();
    token = get_token();
    if (token->type != TCLOSEPARENTHESIS)
      error(") expected");
  } else if ((token + 1)->type == TNUMBER) {
    token = get_token();
    printf("\tmovl $%d, %%eax\n", token->int_value);
  } else if ((token + 1)->type == TIDENTIFIER) {
    token = get_token();

    if ((token + 1)->type == TOPENPARENTHESIS) {
      char *func = token->identifier;
      token = get_token();

      int i = 0;
      while ((token + 1)->type != TCLOSEPARENTHESIS) {
        if (i > 0) {
          if ((token + 1)->type != TCOMMA)
            error(", expected");
          token = get_token();
        }
        read_expr();
        printf("\tmov %%rax, %%%s\n", registers[i]);
        i++;
      }
      token = get_token();
      if (token->type != TCLOSEPARENTHESIS)
        error(") expected");
      printf("\tcall %s\n", func);
    } else {
      long offset = (long)map_get(ident, token->identifier);
      if (!offset)
        error("variable expected");
      printf("\tleaq %ld(%%rbp), %%rax\n", offset);
    }
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

void read_eq_neq_assgin(void) {
  read_add_sub();

  while ((token + 1)->type == TEQ || (token + 1)->type == TNEQ ||
         (token + 1)->type == TASSIGN) {
    token = get_token();
    TokenType op = token->type;
    printf("\tpushq %%rax\n");
    read_add_sub();
    printf("\tmov %%rax, %%rdi\n");
    printf("\tpop %%rax\n");

    if (op == TEQ || op == TNEQ) {
      printf("\tcmpl %%eax, %%edi\n");
      if (op == TEQ) {
        printf("\tsete %%al\n");
      } else if (op == TNEQ) {
        printf("\tsetne %%al\n");
      }
      printf("\tmovzbl %%al, %%eax\n");
    } else if (op == TASSIGN) {
      printf("\tmovl %%edi, (%%rax)\n");
    }
  }
}

void read_expr(void) {
  read_eq_neq_assgin();

  while ((token + 1)->type == TSEMICOLON) {
    token = get_token();
    read_eq_neq_assgin();
  }
}

void read_func(void) {
  if ((token + 1)->type == TIDENTIFIER) {
    token = get_token();
    char *func_name = token->identifier;
    if ((token + 1)->type != TOPENPARENTHESIS)
      error("( expected");
    token = get_token();
    if ((token + 1)->type != TCLOSEPARENTHESIS)
      error(") expected");
    token = get_token();

    printf("%s:\n", func_name);

    if ((token + 1)->type != TOPENBRACE)
      error("{ expected");
    token = get_token();

    func_prologue();

    while (1) {
      if ((token + 1)->type == TCLOSEBRACE || (token + 1)->type == TRETURN)
        break;
      read_expr();
    }

    func_epilogue();
  }
}

static void func_prologue(void) {
  printf("\tpush %%rbp\n");
  printf("\tmov %%rsp, %%rbp\n");

  Vector *ident_keys = map_keys(ident);
  for (int i = 0; i < vector_size(ident_keys); i++) {
    map_push(ident, vector_get(ident_keys, i), (void *)(rbp_offset - 8));
    printf("\tmovl $8, %%edi\n");
    printf("\tsub %%rdi, %%rsp\n");
    rbp_offset -= 8;
  }
}

static void func_epilogue(void) {
  if ((token + 1)->type == TRETURN) {
    token = get_token();
    read_expr();
  }

  for (int i = 0; i < map_size(ident); i++) {
    rbp_offset += 8;
    printf("\tmovl $8, %%edi\n");
    printf("\tadd %%rdi, %%rsp\n");
  }

  if ((token + 1)->type != TCLOSEBRACE)
    error("} expected");
  token = get_token();

  printf("\tpop %%rbp\n");

  printf("\tret\n");
}

void parse(void) {
  if ((token + 1)->type == TTESTVECTOR) {
    printf("main:\n");
    int res = test_vector();
    printf("\tmovl $%d, %%eax\n", res);
  } else if ((token + 1)->type == TTESTMAP) {
    printf("main:\n");
    int res = test_map();
    printf("\tmovl $%d, %%eax\n", res);
  } else {
    while ((token + 1)->type != TEOF)
      read_func();
  }
}
