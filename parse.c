#include "minc.h"

static int tokscmp(Token*, char *);
static Token *next(void);
static Node *read_lvar(void);
static Node *read_return(void);
static Node *read_func_call(void);
static Node *read_if(void);
static Node *read_while(void);
static Node *read_for(void);
static Node *read_decl(void);
static Node *read_ident(void);
static Node *read_term(void);
static Node *read_mul_div(void);
static Node *read_add_sub(void);
static Node *read_shl_shr(void);
static Node *read_expr(void);
static Node *read_comp_stmt(void);
static Node *read_func(void);

static Token *token;

static Token *next(void) { return ++token; }

static Node *comp_stmt_from_expr(void) {
  Node *node = malloc(sizeof(Node));
  node->type = AST_COMP_STMT;
  node->stmts = vector_new();
  vector_push(node->stmts, read_expr());
  token = next(); /* read ";" */

  return node;
}

static int tokscmp(Token *tok, char *string) {
  if (tok->type == TIDENT || tok->type == TPUNCTUATOR) {
    if (!strcmp(tok->sval, string))
      return 1;
  }

  return 0;
}

static Node *read_lvar(void) {
  if ((token + 1)->type != TIDENT) {
    error("TIDENT expected");
    return NULL;
  }

  token = next();
  Node *node = malloc(sizeof(Node));
  node->type = AST_LVAR;
  node->var_name = token->sval;

  return node;
}

static Node *read_return(void) {
  if ((token + 1)->id != KRETURN) {
    error("return expected");
    return NULL;
  }

  token = next();
  Node *node = malloc(sizeof(Node));
  node->type = AST_RETURN;
  node->retval = read_expr();

  return node;
}

static Node *read_func_call(void) {
  if (!((token + 1)->type == TIDENT && tokscmp((token + 2), "("))) {
    error("TIDENT ( expected");
    return NULL;
  }

  token = next();
  Node *node = malloc(sizeof(Node));
  node->type = AST_FUNC_CALL;
  node->func_name = token->sval;
  node->arguments = vector_new();
  token = next();

  int idx = 0;
  Node *expr = read_expr();
  if (expr->type == AST_EXPR && expr->expr->type != UNK) {
    vector_push(node->arguments, expr);
    idx++;
  }

  while (tokscmp((token + 1), ",")) {
    token = next();

    expr = read_expr();
    if (expr->type == AST_EXPR && expr->expr->type != UNK) {
      vector_push(node->arguments, expr);
      idx++;
    } else
      break;
  }

  if (!tokscmp((token + 1), ")"))
    error(") expected");
  token = next();

  return node;
}

static Node *read_if(void) {
  if (!((token + 1)->id == KIF && tokscmp((token + 2), "("))) {
    error("if ( expected");
    return NULL;
  }

  token = next(); /* read "if" */
  token = next(); /* read "(" */
  Node *node = malloc(sizeof(Node));
  node->type = AST_IF;
  node->cond = read_expr();

  if (!tokscmp((token + 1), ")"))
    error(") expected");
  token = next();

  if (tokscmp((token + 1), "{"))
    node->then = read_comp_stmt();
  else
    node->then = comp_stmt_from_expr();

  if ((token + 1)->id == KELSE) {
    token = next();
    if (tokscmp((token + 1), "{"))
      node->els = read_comp_stmt();
    else
      node->els = comp_stmt_from_expr();
  } else {
    node->els = NULL;
  }

  node->init = NULL;
  node->incdec = NULL;

  return node;
}

static Node *read_while(void) {
  if (!((token + 1)->id == KWHILE && tokscmp((token + 2), "("))) {
    error("while ( expected");
    return NULL;
  }

  token = next(); /* read "while" */
  token = next(); /* read "(" */
  Node *node = malloc(sizeof(Node));
  node->type = AST_WHILE;
  node->cond = read_expr();

  if (!tokscmp((token + 1), ")"))
    error(") expected");
  token = next();

  if (tokscmp((token + 1), "{"))
    node->then = read_comp_stmt();
  else
    node->then = comp_stmt_from_expr();

  node->init = NULL;
  node->incdec = NULL;
  node->els = NULL;

  return node;
}

static Node *read_for(void) {
  if (!((token + 1)->id == KFOR && tokscmp((token + 2), "("))) {
    error("for ( expected");
    return NULL;
  }

  token = next(); /* read "for" */
  token = next(); /* read "(" */
  Node *node = malloc(sizeof(Node));
  node->type = AST_FOR;
  node->init = read_expr();
  token = next(); /* read ";" */
  node->cond = read_expr();
  token = next(); /* read ";" */
  node->incdec = read_expr();

  if (!tokscmp((token + 1), ")"))
    error(") expected");
  token = next();

  if (tokscmp((token + 1), "{"))
    node->then = read_comp_stmt();
  else
    node->then = comp_stmt_from_expr();

  node->els = NULL;

  return node;
}

static Node *read_decl(void) {
  /* FIXME: Allow other types */
  if (!((token + 1)->type == TKEYWORD && (token + 1)->id == KINT)) {
    error("int expected");
    return NULL;
  }
  token = next();

  if (!((token + 1)->type == TIDENT))
    error("TINDENT expected");

  Node *node = malloc(sizeof(Node));
  node->type = AST_DECL;
  node->declvar = read_expr();

  return node;
}

static Node *read_ident(void) {
  if (!((token + 1)->type == TIDENT)) {
    error("TIDENT expected");
    return NULL;
  }

  if (tokscmp((token + 2), "("))
    return read_func_call();
  else
    return read_lvar();
}

static Node *read_term(void) {
  Node *node = malloc(sizeof(Node));

  if (tokscmp((token + 1), "(")) {
    token = next();
    node = read_expr();
    if (!tokscmp((token + 1), ")"))
      error(") expected");
    token = next();
  } else if ((token + 1)->type == TNUMBER) {
    token = next();
    node->type = AST_LITERAL;
    node->int_value = atoi(token->sval);
  } else if ((token + 1)->type == TIDENT) {
    node = read_ident();
  }

  return node;
}

static Node *read_mul_div(void) {
  Node *node = read_term();

  while (tokscmp((token + 1), "*")
      || tokscmp((token + 1), "/")
      || tokscmp((token + 1), "%")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    if (tokscmp(token, "*"))
      tmp->type = OP_MUL;
    else if (tokscmp(token, "/"))
      tmp->type = OP_DIV;
    else if (tokscmp(token, "%"))
      tmp->type = OP_REM;

    tmp->right = read_term();
    
    node = tmp;
  }

  return node;
}

static Node *read_add_sub(void) {
  Node *node = read_mul_div();

  while (tokscmp((token + 1), "+") || tokscmp((token + 1), "-")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    if (tokscmp(token, "+"))
      tmp->type = OP_ADD;
    else if (tokscmp(token, "-"))
      tmp->type = OP_SUB;

    tmp->right = read_mul_div();

    node = tmp;
  }

  return node;
}

static Node *read_shl_shr(void) {
  Node *node = read_add_sub();

  while (tokscmp((token + 1), "<<") || tokscmp((token + 1), ">>")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    if (tokscmp(token, "<<"))
      tmp->type = OP_SHL;
    else if (tokscmp(token, ">>"))
      tmp->type = OP_SHR;

    tmp->right = read_add_sub();

    node = tmp;
  }

  return node;
}

static Node *read_eq_and_neq(void) {
  Node *node = read_shl_shr();

  while (tokscmp((token + 1), "==") || tokscmp((token + 1), "!=")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    if (tokscmp(token, "=="))
      tmp->type = OP_EQ;
    else if (tokscmp(token, "!="))
      tmp->type = OP_NEQ;

    tmp->right = read_shl_shr();

    node = tmp;
  }

  return node;
}

static Node *read_assgin(void) {
  Node *node = read_eq_and_neq();

  while (tokscmp((token + 1), "=")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    tmp->type = OP_ASSGIN;

    tmp->right = read_eq_and_neq();

    node = tmp;
  }

  return node;
}

static Node *read_expr(void) {
  if ((token + 1)->id == KRETURN) {
    return read_return();
  } else if ((token + 1)->id == KIF) {
    return read_if();
  } else if ((token + 1)->id == KWHILE) {
    return read_while();
  } else if ((token + 1)->id == KFOR) {
    return read_for();
  } else if ((token + 1)->id == KINT) {
    return read_decl();
  } else {
    Node *node = malloc(sizeof(Node));
    node->type = AST_EXPR;
    node->expr = read_assgin();
    return node;
  }
}

static Node *read_comp_stmt(void) {
  if (!tokscmp((token + 1), "{"))
    error("{ expected");
  token = next();

  Node *node = malloc(sizeof(Node));
  node->type = AST_COMP_STMT;
  node->stmts = vector_new();

  Node *expr = read_expr();
  if (expr->type == AST_EXPR && expr->expr->type == UNK) {
    /* Do nothing */
  } else
    vector_push(node->stmts, expr);

  while (tokscmp((token + 1), ";") || !tokscmp((token + 1), "}")) {
    if (tokscmp((token + 1), ";"))
      token = next();

    expr = read_expr();
    if (expr->type == AST_EXPR && expr->expr->type == UNK) {
      /* Do nothing */
    } else
      vector_push(node->stmts, expr);
  }

  if (!tokscmp((token + 1), "}"))
    error("} expected");
  token = next();

  return node;
}

static Node *read_func(void) {
  /* FIXME: Allow other types */
  if (!((token + 1)->type == TKEYWORD && (token + 1)->id == KINT))
    error("int expected");
  token = next();

  if ((token + 1)->type != TIDENT)
    error("TIDENT expected");
  token = next();

  char *func_name = token->sval;

  if (!tokscmp((token + 1), "("))
    error("( expected");
  token = next();

  Node *node = malloc(sizeof(Node));
  node->type = AST_FUNC;
  node->func_name = func_name;
  node->arguments = vector_new();

  /* FIXME: Allow other types */
  if (!tokscmp((token + 1), ")")) {
    if (!((token + 1)->type == TKEYWORD && (token + 1)->id == KINT))
      error("int expected");
    token = next();
  }

  int idx = 0;
  Node *expr = read_expr();
  if (expr->type == AST_EXPR && expr->expr->type != UNK) {
    vector_push(node->arguments, expr);
    idx++;
  }

  while (tokscmp((token + 1), ",")) {
    token = next();

    /* FIXME: Allow other types */
    if (!((token + 1)->type == TKEYWORD && (token + 1)->id == KINT))
      error("int expected");
    token = next();

    expr = read_expr();
    if (expr->type == AST_EXPR && expr->expr->type != UNK) {
      vector_push(node->arguments, expr);
      idx++;
    } else
      break;
  }

  if (!tokscmp((token + 1), ")"))
    error(") expected");
  token = next();


  node->body = read_comp_stmt();

  return node;
}

void parse_init(void) {
  token = tokens - 1;
}

Vector *parse_toplevel(void) {
  Vector *toplevels = vector_new();
  while ((token + 1)->type != TEOF)
    vector_push(toplevels, read_func());

  return toplevels;
}
