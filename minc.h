#ifndef MINC_H
#define MINC_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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
struct Token *token;

/* debug.c */
void error(char *fmt, ...);
char *dump_token(Token *p);

/* parse.c */
struct Token *get_token(void);
void tokenize(void);
void read_term(void);
void read_mul_div(void);
void read_add_sub(void);
void read_eq_neq(void);
void read_expr(void);
void parse(void);

#endif /* MINC_H */
