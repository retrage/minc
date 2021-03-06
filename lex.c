#include "minc.h"

static int src_pos;
static int token_len;
static int token_pos;

static void read_src(void) {
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
}

static void next(void) {
  if (token_pos >= token_len) {
    token_len *= 2;
    tokens = realloc(tokens, token_len);
    if (!tokens)
      error("realloc failed");
  }
  token_pos++;
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
  char *literal = malloc(sizeof(char) * 1024);
  int idx = 0;

  while (1) {
    if (is_digit(source[src_pos + 1])) {
      literal[idx] = source[src_pos + 1];
      src_pos++;
      idx++;
    } else {
      if (idx > 0) {
        literal[idx] = '\0';
        tokens[token_pos].type = TNUMBER;
        tokens[token_pos].sval = literal;
        next();
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
        if (!strcmp(string, "test_vector")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KTESTVECTOR;
        } else if (!strcmp(string, "test_map")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KTESTMAP;
        } else if (!strcmp(string, "return")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KRETURN;
        } else if (!strcmp(string, "if")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KIF;
        } else if (!strcmp(string, "else")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KELSE;
        } else if (!strcmp(string, "while")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KWHILE;
        } else if (!strcmp(string, "do")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KDO;
        } else if (!strcmp(string, "for")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KFOR;
        } else if (!strcmp(string, "goto")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KGOTO;
        } else if (!strcmp(string, "break")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KBREAK;
        } else if (!strcmp(string, "continue")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KCONTINUE;
        } else if (!strcmp(string, "int")) {
          tokens[token_pos].type = TKEYWORD;
          tokens[token_pos].id = KINT;
        } else {
          if (strlen(string) > 128)
            error("string too long");
          tokens[token_pos].type = TIDENT;
          tokens[token_pos].sval = malloc(sizeof(char) * 128);
          strcpy(tokens[token_pos].sval, string);
        }
        next();
        return;
      }
    }
  }
}

static void read_symbol(void) {
  char symbol[4];

  switch (source[src_pos + 1]) {
    case '+':
      if (source[src_pos + 1] == '+' && source[src_pos + 2] == '+') {
        tokens[token_pos].type = TPUNCTUATOR;
        tokens[token_pos].sval = malloc(sizeof(char) * 4);
        strcpy(tokens[token_pos].sval, "++");
        src_pos++;
        break;
      }
    case '-':
      if (source[src_pos + 1] == '-' && source[src_pos + 2] == '-') {
        tokens[token_pos].type = TPUNCTUATOR;
        tokens[token_pos].sval = malloc(sizeof(char) * 4);
        strcpy(tokens[token_pos].sval, "--");
        src_pos++;
        break;
      }
    case '*':
    case '/':
    case '%':
    case '&':
      /* Tokenize && */
      if (source[src_pos + 1] == '&' && source[src_pos + 2] == '&') {
        tokens[token_pos].type = TPUNCTUATOR;
        tokens[token_pos].sval = malloc(sizeof(char) * 4);
        strcpy(tokens[token_pos].sval, "&&");
        src_pos++;
        break;
      }
    case '^':
    case '|':
      /* Tokenize || */
      if (source[src_pos + 1] == '|' && source[src_pos + 2] == '|') {
        tokens[token_pos].type = TPUNCTUATOR;
        tokens[token_pos].sval = malloc(sizeof(char) * 4);
        strcpy(tokens[token_pos].sval, "||");
        src_pos++;
        break;
      }
      /* Tokenize *= /= %= += -= &= ^= |= */
      if (source[src_pos + 2] == '=') {
        symbol[0] = source[src_pos + 1];
        symbol[1] = source[src_pos + 2];
        symbol[2] = '\0';
        tokens[token_pos].type = TPUNCTUATOR;
        tokens[token_pos].sval = malloc(sizeof(char) * 4);
        strcpy(tokens[token_pos].sval, symbol);
        src_pos++;
        break;
      }
    case '(':
    case ')':
    case ';':
    case ',':
    case '{':
    case '}':
    case '?':
    case ':':
    case '~':
    case '!':
      if (source[src_pos + 1] == '!' && source[src_pos + 2] == '=') {
        tokens[token_pos].type = TPUNCTUATOR;
        tokens[token_pos].sval = malloc(sizeof(char) * 4);
        strcpy(tokens[token_pos].sval, "!=");
        src_pos++;
        break;
      }
      tokens[token_pos].type = TPUNCTUATOR;
      symbol[0] = source[src_pos + 1];
      symbol[1] = '\0';
      tokens[token_pos].sval = malloc(sizeof(char) * 2);
      strcpy(tokens[token_pos].sval, symbol);
      break;
    case '=':
      tokens[token_pos].type = TPUNCTUATOR;
      tokens[token_pos].sval = malloc(sizeof(char) * 4);
      if (source[src_pos + 2] == '=') {
        strcpy(tokens[token_pos].sval, "==");
        src_pos++;
      } else {
        strcpy(tokens[token_pos].sval, "=");
      }
      break;
    case '<':
      tokens[token_pos].type = TPUNCTUATOR;
      tokens[token_pos].sval = malloc(sizeof(char) * 4);
      if (source[src_pos + 2] == '<') {
        if (source[src_pos + 3] == '=') {
          strcpy(tokens[token_pos].sval, "<<=");
          src_pos += 2;
        } else {
          strcpy(tokens[token_pos].sval, "<<");
          src_pos++;
        }
      } else if (source[src_pos + 2] == '=') {
        strcpy(tokens[token_pos].sval, "<=");
        src_pos++;
      } else {
        strcpy(tokens[token_pos].sval, "<");
      }
      break;
    case '>':
      tokens[token_pos].type = TPUNCTUATOR;
      tokens[token_pos].sval = malloc(sizeof(char) * 4);
      if (source[src_pos + 2] == '>') {
        if (source[src_pos + 3] == '=') {
          strcpy(tokens[token_pos].sval, ">>=");
          src_pos += 2;
        } else {
          strcpy(tokens[token_pos].sval, ">>");
          src_pos++;
        }
      } else if (source[src_pos + 2] == '=') {
        strcpy(tokens[token_pos].sval, ">=");
        src_pos++;
      } else {
        strcpy(tokens[token_pos].sval, ">");
      }
      break;
    default:
      return;
  }
  next();
}

static void tokenize_init(void) {
  token_len = 128;
  tokens = malloc(sizeof(Token) * token_len);
  src_pos = -1;
  token_pos = 0;
}

void tokenize(void) {
  tokenize_init();
  read_src();
 
  while (1) {
    read_number();
    read_string();
    read_symbol();

    if (source[src_pos + 1] == EOF) {
      tokens[token_pos].type = TEOF;
      break;
    }
    src_pos++;
  }
}
