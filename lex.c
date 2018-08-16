#include "minc.h"

int src_len;
int src_pos;
Token *next_token;

void read_src(void) {
  size_t i = 0;
  size_t src_len = 128;
  source = malloc(sizeof(char) * src_len);
  if (!source)
    error("malloc failed");

  while (1) {
    if (i >= src_len) {
      src_len *= 2;
      source = realloc(source, src_len);
      if (!source)
        error("realloc failed");
    }

    source[i] = getchar();
    if (source[i] == EOF)
      break;
    i++;
  }

  src_len = (size_t)i;
}

static int is_digit(char c) {
  if (c >= '0' && c <= '9')
    return 1;
  else
    return 0;
}

static int is_letter(char c) {
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
    return 1;
  else
    return 0;
}

static void read_number(void) {
  char literal[1024];
  int idx = 0;

  while (1) {
    if (is_digit(source[src_pos + 1])) {
      literal[idx] = source[src_pos + 1];
      src_pos++;
      idx++;
    } else {
      if (idx > 0) {
        literal[idx] = '\0';
        next_token->type = TNUMBER;
        next_token->int_value = atoi(literal);
        next_token++;
      }
      return;
    }
  }
}

static void read_string(void) {
  char string[1024];
  int idx = 0;

  if (!is_letter(source[src_pos + 1]))
    return;
  else {
    string[idx] = source[src_pos + 1];
    src_pos++;
    idx++;

    while (1) {
      if (is_letter(source[src_pos + 1]) || is_digit(source[src_pos + 1])) {
        string[idx] = source[src_pos + 1];
        src_pos++;
        idx++;
      } else {
        string[idx] = '\0';
        if (!strcmp(string, "test_vector"))
          next_token->type = TTESTVECTOR;
        else if (!strcmp(string, "test_map"))
          next_token->type = TTESTMAP;
        else if (!strcmp(string, "return"))
          next_token->type = TRETURN;
        else {
          next_token->type = TIDENTIFIER;
          strcpy(next_token->identifier, string);
          map_push(ident, next_token->identifier, (void *)0);
        }
        next_token++;
        return;
      }
    }
  }
}

static void read_symbol(void) {
  switch (source[src_pos + 1]) {
    case '+':
      next_token->type = TADD;
      break;
    case '-':
      next_token->type = TSUB;
      break;
    case '*':
      next_token->type = TMUL;
      break;
    case '/':
      next_token->type = TDIV;
      break;
    case '(':
      next_token->type = TOPENPARENTHESIS;
      break;
    case ')':
      next_token->type = TCLOSEPARENTHESIS;
      break;
    case ';':
      next_token->type = TSEMICOLON;
      break;
    case ',':
      next_token->type = TCOMMA;
      break;
    case '{':
      next_token->type = TOPENBRACE;
      break;
    case '}':
      next_token->type = TCLOSEBRACE;
      break;
    case '=':
      if (source[src_pos + 2] == '=') {
        src_pos++;
        next_token->type = TEQ;
      } else {
        next_token->type = TASSIGN;
      }
      break;
    case '!':
      if (source[src_pos + 2] == '=') {
        src_pos++;
        next_token->type = TNEQ;
      } else {
        error("!= expected");
      }
      break;
    default:
      return;
  }
  next_token++;
}

static void init_tokenizer(void) {
  src_pos = -1;
  src_len = 0;
  next_token = tokens;
}

void tokenize(void) {
  init_tokenizer();
  read_src();
 
  while (1) {
    read_number();
    read_string();
    read_symbol();

    if (source[src_pos + 1] == EOF) {
      next_token->type = TEOF;
      break;
    }
    src_pos++;
  }
}
