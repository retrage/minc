#ifndef MINC_H
#define MINC_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
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
} TokenType;

typedef struct {
  TokenType type;
  int int_value;
} Token;

Token tokens[100];
Token *token;

/* debug.c */
void error(char *fmt, ...);
char *dump_token(Token *p);

/* parse.c */
Token *get_token(void);
void tokenize(void);
void read_term(void);
void read_mul_div(void);
void read_add_sub(void);
void read_eq_neq(void);
void read_expr(void);
void parse(void);

#endif /* MINC_H */
