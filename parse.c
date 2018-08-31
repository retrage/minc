#include "minc.h"

static int tokscmp(Token*, char *);
static Token *next(void);
static Type *read_type(void);
static Node *comp_stmt_from_expr(void);
static Node *read_lvar(void);
static Node *read_return(void);
static Node *read_func_call(void);
static Node *read_if(void);
static Node *read_while(void);
static Node *read_for(void);
static Node *read_decl(void);
static Node *read_ident(void);
static Node *read_term(void);
static Node *read_unary(void);
static Node *read_mul_div(void);
static Node *read_add_sub(void);
static Node *read_shl_shr(void);
static Node *read_lt_gt_le_ge(void);
static Node *read_eq_neq(void);
static Node *read_and(void);
static Node *read_xor(void);
static Node *read_or(void);
static Node *read_log_and(void);
static Node *read_log_or(void);
static Node *read_ternary(void);
static Node *read_assgin(void);
static Node *read_expr(void);
static Node *read_comp_stmt(void);
static Node *read_func(void);

static Token *token;

static Token *next(void) { return ++token; }

static Type *read_type(void) {
  /* FIXME: Allow other types */
  if (!((token + 1)->type == TKEYWORD && (token + 1)->id == KINT)) {
    return NULL;
  }
  token = next();

  Type *ty = malloc(sizeof(Type));
  ty->ty = TYINT;
  ty->ptrof = NULL;

  while (tokscmp((token + 1), "*")) {
    token = next();

    Type *tmp = malloc(sizeof(Type));
    tmp->ty = TYPTR;
    tmp->ptrof = ty;

    ty = tmp;
  }

  return ty;
}

static int tokscmp(Token *tok, char *string) {
  if (tok->type == TIDENT || tok->type == TPUNCTUATOR) {
    if (!strcmp(tok->sval, string))
      return 1;
  }

  return 0;
}

static Node *comp_stmt_from_expr(void) {
  Node *node = malloc(sizeof(Node));
  node->type = AST_COMP_STMT;
  node->stmts = vector_new();
  vector_push(node->stmts, read_expr());
  if (tokscmp((token + 1), ";"))
    token = next(); /* read ";" */

  return node;
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
  Type *ty = read_type();
  if (!ty)
    error("type expected");

  Node *node = malloc(sizeof(Node));
  node->type = AST_DECL;
  node->declvar = read_expr();
  node->declvar->expr->ty = ty;

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

static Node *read_unary(void) {
  Node *node;

  if (tokscmp((token + 1), "&")) {
    token = next();

    node = malloc(sizeof(Node));
    node->type = AST_ADDR;
    node->operand = read_expr();
  } else if (tokscmp((token + 1), "*")) {
    token = next();

    node = malloc(sizeof(Node));
    node->type = AST_DEREF;
    node->operand =  read_expr();
  } else
    node = read_term();

  return node;
}

static Node *read_mul_div(void) {
  Node *node = read_unary();

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

    tmp->right = read_unary();
    
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

static Node *read_lt_gt_le_ge(void) {
  Node *node = read_shl_shr();

  while (tokscmp((token + 1), "<")
      || tokscmp((token + 1), ">")
      || tokscmp((token + 1), "<=")
      || tokscmp((token + 1), ">=")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    if (tokscmp(token, "<"))
      tmp->type = OP_LT;
    else if (tokscmp(token, ">"))
      tmp->type = OP_GT;
    else if (tokscmp(token, "<="))
      tmp->type = OP_LE;
    else if (tokscmp(token, ">="))
      tmp->type = OP_GE;

    tmp->right = read_shl_shr();

    node = tmp;
  }

  return node;
}

static Node *read_eq_neq(void) {
  Node *node = read_lt_gt_le_ge();

  while (tokscmp((token + 1), "==") || tokscmp((token + 1), "!=")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    if (tokscmp(token, "=="))
      tmp->type = OP_EQ;
    else if (tokscmp(token, "!="))
      tmp->type = OP_NEQ;

    tmp->right = read_lt_gt_le_ge();

    node = tmp;
  }

  return node;
}

static Node *read_and(void) {
  Node *node = read_eq_neq();

  while (tokscmp((token + 1), "&")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    if (tokscmp(token, "&"))
      tmp->type = OP_AND;

    tmp->right = read_eq_neq();

    node = tmp;
  }

  return node;
}
static Node *read_xor(void) {
  Node *node = read_and();

  while (tokscmp((token + 1), "^")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    if (tokscmp(token, "^"))
      tmp->type = OP_XOR;

    tmp->right = read_and();

    node = tmp;
  }

  return node;
}

static Node *read_or(void) {
  Node *node = read_xor();

  while (tokscmp((token + 1), "|")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    if (tokscmp(token, "|"))
      tmp->type = OP_OR;

    tmp->right = read_xor();

    node = tmp;
  }

  return node;
}

static Node *read_log_and(void) {
  Node *node = read_or();

  while (tokscmp((token + 1), "&&")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    if (tokscmp(token, "&&"))
      tmp->type = OP_LOG_AND;

    tmp->right = read_or();

    node = tmp;
  }

  return node;
}

static Node *read_log_or(void) {
  Node *node = read_log_and();

  while (tokscmp((token + 1), "||")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;
    token = next();

    if (tokscmp(token, "||"))
      tmp->type = OP_LOG_OR;

    tmp->right = read_log_and();

    node = tmp;
  }

  return node;
}

static Node *read_ternary(void) {
  Node *node = read_log_or();

  if (tokscmp((token + 1), "?")) {
    token = next(); /* read "?" */
    Node *tmp = malloc(sizeof(Node));
    tmp->type = AST_IF;
    tmp->cond = node;
    tmp->then = comp_stmt_from_expr();

    if (!tokscmp((token + 1), ":"))
      error(": expected, got %s", tok2s((token + 1)));
    token = next(); /* read : */

    tmp->els = comp_stmt_from_expr();

    tmp->init = NULL;
    tmp->incdec = NULL;

    node = tmp;
  }

  return node;
}

static Node *read_assgin(void) {
  Node *node = read_ternary();

  while (tokscmp((token + 1), "=")
      || tokscmp((token + 1), "*=")
      || tokscmp((token + 1), "/=")
      || tokscmp((token + 1), "%=")
      || tokscmp((token + 1), "+=")
      || tokscmp((token + 1), "-=")
      || tokscmp((token + 1), "<<=")
      || tokscmp((token + 1), ">>=")
      || tokscmp((token + 1), "&=")
      || tokscmp((token + 1), "^=")
      || tokscmp((token + 1), "|=")) {
    Node *tmp = malloc(sizeof(Node));
    tmp->left = node;

    if (!tokscmp((token + 1), "=")) {
      Node *left = malloc(sizeof(Node));
      memcpy(left, tmp->left, sizeof(Node));
      Node *op = malloc(sizeof(Node));
      op->left = left;
      if (tokscmp((token + 1), "*="))
        op->type = OP_MUL;
      else if (tokscmp((token + 1), "/="))
        op->type = OP_DIV;
      else if (tokscmp((token + 1), "%="))
        op->type = OP_REM;
      else if (tokscmp((token + 1), "+="))
        op->type = OP_ADD;
      else if (tokscmp((token + 1), "-="))
        op->type = OP_SUB;
      else if (tokscmp((token + 1), "<<="))
        op->type = OP_SHL;
      else if (tokscmp((token + 1), ">>="))
        op->type = OP_SHR;
      else if (tokscmp((token + 1), "&="))
        op->type = OP_AND;
      else if (tokscmp((token + 1), "^="))
        op->type = OP_XOR;
      else if (tokscmp((token + 1), "|="))
        op->type = OP_OR;

      token = next();
      op->right = read_ternary();
      tmp->type = OP_ASSGIN;
      tmp->right = op;
    } else {
      token = next();
      tmp->type = OP_ASSGIN;
      tmp->right = read_ternary();
    }

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
  /*  FIXME: Save return type */
  read_type();

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

  int idx = 0;
  Type *ty = read_type();
  Node *expr = read_expr();
  if (!ty)
    goto end_read_args;

  if (expr->type == AST_EXPR && expr->expr->type == AST_LVAR) {
    expr->expr->ty = ty;
    vector_push(node->arguments, expr);
    idx++;
  }

  while (tokscmp((token + 1), ",")) {
    token = next();

    ty = read_type();
    expr = read_expr();
    if (!ty)
      goto end_read_args;

    if (ty && expr->type == AST_EXPR && expr->expr->type == AST_LVAR) {
      expr->expr->ty = ty;
      vector_push(node->arguments, expr);
      idx++;
    } else
      break;
  }

end_read_args:
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
