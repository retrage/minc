#ifndef MINC_H
#define MINC_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  TTESTVECTOR,
  TTESTMAP,
} TokenType;

typedef struct {
  TokenType type;
  int int_value;
} Token;

typedef struct {
  int vec_len;
  int buf_len;
  void **buf;
} Vector;

typedef struct {
  int map_len;
  Vector *keys;
  Vector *values;
} Map;

Token tokens[100];
Token *token;

/* vector.c */
Vector *vector_new(void);
void vector_push(Vector *, void *);
void *vector_get(Vector *, size_t);
size_t vector_size(Vector *);

/* map.c */
Map *map_new(void);
void map_push(Map *, char *, void *);
void *map_get(Map *, char *);
size_t map_size(Map *);
Vector *map_keys(Map *);

/* debug.c */
void error(char *, ...);
char *dump_token(Token *);

/* parse.c */
Token *get_token(void);
void tokenize(void);
void read_term(void);
void read_mul_div(void);
void read_add_sub(void);
void read_eq_neq(void);
void read_expr(void);
void parse(void);

/* test.c */
int test_vector(void);
int test_map(void);

#endif /* MINC_H */
