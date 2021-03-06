#ifndef MINC_H
#define MINC_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int len;
  int buf_len;
  char *buf;
} Buffer;

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

/* Token type */
enum {
  TEOF,
  TNUMBER,
  TIDENT,
  TKEYWORD,
  TPUNCTUATOR,
};

/* Keyword */
enum {
  KTESTVECTOR,
  KTESTMAP,
  KRETURN,
  KIF,
  KELSE,
  KWHILE,
  KDO,
  KFOR,
  KGOTO,
  KBREAK,
  KCONTINUE,
  KINT,
};

typedef struct {
  int type;
  int id;
  char *sval;
} Token;

enum {
  UNK = 0,
  AST_FUNC,
  AST_FUNC_CALL,
  AST_COMP_STMT,
  AST_LITERAL,
  AST_LVAR,
  AST_EXPR,
  AST_RETURN,
  AST_IF,
  AST_WHILE,
  AST_DO_WHILE,
  AST_FOR,
  AST_GOTO,
  AST_LABEL,
  AST_BREAK,
  AST_CONTINUE,
  AST_DECL,
  AST_ADDR,
  AST_DEREF,
  AST_POS,
  AST_NEG,
  AST_COMP,
  AST_LOG_NEG,
  AST_PRE_INC,
  AST_PRE_DEC,
  AST_TESTVECTOR,
  AST_TESTMAP,
  OP_LT,
  OP_GT,
  OP_LE,
  OP_GE,
  OP_EQ,
  OP_NEQ,
  OP_AND,
  OP_XOR,
  OP_OR,
  OP_LOG_AND,
  OP_LOG_OR,
  OP_ASSGIN,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_REM,
  OP_SHL,
  OP_SHR,
};

enum {
  TYUNK,
  TYINT,
  TYPTR,
};

typedef struct Type {
  int ty;
  struct Type *ptrof;
  int size;
  int offset;
} Type;

typedef struct Env {
  Map *lvars;
  Map *labels;
} Env;

typedef struct Node {
  int type;
  char *label;
  union {
    /* Function declaration and Call */
    struct {
      char *func_name;
      Env *env;
      struct Node *body;
      Vector *arguments;
    };
    /* Compound statement */
    Vector *stmts;
    /* Binary operator */
    struct {
      struct Node *left;
      struct Node *right;
      /* Pointer Add, Sub */
      Type *operand_ty;
      /* Log and, Log or */
      char *label_false;
      char *label_true;
    };
    /* Integer */
    int int_value;
    /* Label */
    char *str_value;
    /* Local variable */
    struct {
      char *var_name;
      Type *ty;
    };
    /* Return */
    struct {
      struct Node *retval;
      char *retlabel;
    };
    /* Expression */
    struct Node *expr;
    /* If statement, While, For */
    struct {
      struct Node *init;
      struct Node *cond;
      struct Node *incdec;
      struct Node *then;
      struct Node *els;
    };
    /* Goto, Break, Continue */
    char *dest;
    /* Declaration */
    struct {
      struct Node *declvar;
    };
    /* Unary operator */
    struct {
      struct Node *operand;
    };
  };
} Node;

char *source;
Token *tokens;

/* buffer.c */
Buffer *buffer_new(void);
void buffer_write(Buffer *, char);
void buffer_append(Buffer *, char *, int);
char *buffer_body(Buffer *buf);
size_t buffer_size(Buffer *);
char *vformat(char *, va_list);
char *format(char *, ...);

/* vector.c */
Vector *vector_new(void);
void vector_push(Vector *, void *);
void *vector_pop(Vector *);
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
char *tok2s(Token *);
void dump_tokens(void);
char *node2s(Node *);

/* lex.c */
void tokenize(void);

/* parse.c */
Vector *parse_toplevel(void);

/* analyze.c */
void analyze_toplevel(Vector *);

/* gen.c */
void emit_toplevel(Vector *);

/* test.c */
int test_vector(void);
int test_map(void);

/* util.c */
int calc_offset(Map *);

#endif /* MINC_H */
