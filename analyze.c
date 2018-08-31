#include "minc.h"

static void analyze_func_call(Node *, Map *);
static void analyze_if(Node *, Map *);
static void analyze_while(Node *, Map *);
static void analyze_for(Node *, Map *);
static void analyze_decl(Node *, Map *);
static void analyze_addr(Node *, Map *);
static void analyze_deref(Node *, Map *);
static void analyze_literal(Node *, Map *);
static void analyze_lvar(Node *, Map *);
static void analyze_op(Node *, Map *);
static void analyze_expr(Node *, Map *);
static void analyze_comp_stmt(Node *, Map *);
static void analyze_func(Node *, Map *);

static void analyze_func_call(Node *node, Map *env) {
  if (node->type != AST_FUNC_CALL)
    error("internal error");

  if (vector_size(node->arguments) > 6)
    error("too many arguments");

  /* FIXME: check number of arguments */
  for (int i = 0; i < vector_size(node->arguments); i++) {
    Node *arg = vector_get(node->arguments, i);
    analyze_expr(arg, env);
  }
}

static void analyze_if(Node *node, Map *env) {
  if (node->type != AST_IF)
    error("internal error");

  analyze_expr(node->cond, env);
  analyze_comp_stmt(node->then, env);
  if (node->els)
    analyze_comp_stmt(node->els, env);
}

static void analyze_while(Node *node, Map *env) {
  if (node->type != AST_WHILE)
    error("internal error");

  analyze_expr(node->cond, env);
  analyze_comp_stmt(node->then, env);
}

static void analyze_for(Node *node, Map *env) {
  if (node->type != AST_FOR)
    error("internal error");

  analyze_expr(node->init, env);
  analyze_expr(node->cond, env);
  analyze_expr(node->incdec, env);
  analyze_comp_stmt(node->then, env);
}

static void analyze_decl(Node *node, Map *env) {
  if (node->type != AST_DECL)
    error("internal error");

  Node *declvar = node->declvar->expr;
  Vector *lvars = map_keys(env);
  for (int i = 0; i < vector_size(lvars); i++) {
    if (!strcmp(declvar->var_name, vector_get(lvars, i)))
      error("redeclaration of %s", declvar->var_name);
  }

  /* FIXME: calculate offset by size of type */
  switch (declvar->ty->ty) {
    case TYINT:
    case TYPTR:
      declvar->ty->offset = -8 * (map_size(env) + 1);
      break;
    case TYUNK:
    default:
      error("internal error");
  }

  map_push(env, declvar->var_name, (void *)declvar->ty);
}

static void analyze_addr(Node *node, Map *env) {
  if (node->type != AST_ADDR)
    error("internal error");

  Node *expr = node->operand->expr;
  if (expr->type != AST_LVAR)
    error("lvalue required as unary '&' operand");

  Type *ty = map_get(env, expr->var_name);
  if (ty)
    expr->ty = ty;
  else
    error("%s undeclared", expr->var_name);
}

static void analyze_deref(Node *node, Map *env) {
  if (node->type != AST_DEREF)
    error("internal error");

  /* FIXME: check if operand represents address */
  Node *expr = node->operand->expr;
  Type *ty = map_get(env, expr->var_name);
  if (ty)
    expr->ty = ty;
  else
    error("%s undeclared", expr->var_name);
}

static void analyze_literal(Node *node, Map *env) {
  if (node->type != AST_LITERAL)
    error("internal error");
}

static void analyze_lvar(Node *node, Map *env) {
  if (node->type != AST_LVAR)
    error("internal error");

  Type *ty = map_get(env, node->var_name);
  if (ty)
    node->ty = ty;
  else
    error("%s undeclared", node->var_name);
}

static void analyze_op(Node *node, Map *env) {
  switch (node->type) {
    case OP_ASSGIN:
      /* FIXME: check if valid lvalue */
      if (node->left->type != AST_LVAR)
        error("lvalue must be variable");
      analyze_expr(node->left, env);
      analyze_expr(node->right, env);
      break;
    case OP_LT:
    case OP_GT:
    case OP_LE:
    case OP_GE:
    case OP_EQ:
    case OP_NEQ:
    case OP_AND:
    case OP_XOR:
    case OP_OR:
    case OP_LOG_AND:
    case OP_LOG_OR:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_REM:
    case OP_SHL:
    case OP_SHR:
      analyze_expr(node->left, env);
      analyze_expr(node->right, env);
      break;
    default:
      error("internal error %s", node2s(node));
  }
}

static void analyze_expr(Node *node, Map *env) {
  if (node->type == AST_RETURN) {
    analyze_expr(node->retval, env);
    return;
  } else if (node->type == AST_EXPR) {
    analyze_expr(node->expr, env);
    return;
  }

  switch (node->type) {
    case AST_FUNC_CALL: analyze_func_call(node, env); break;
    case AST_COMP_STMT: analyze_comp_stmt(node, env); break;
    case AST_LITERAL:   analyze_literal(node, env);   break;
    case AST_LVAR:      analyze_lvar(node, env);      break;
    case AST_EXPR:      analyze_expr(node, env);      break;
    case AST_RETURN:    error("return must be unique in expr"); break;
    case AST_IF:        analyze_if(node, env);        break;
    case AST_WHILE:     analyze_while(node, env);     break;
    case AST_FOR:       analyze_for(node, env);       break;
    case AST_DECL:      analyze_decl(node, env);      break;
    case AST_ADDR:      analyze_addr(node, env);      break;
    case AST_DEREF:     analyze_deref(node, env);     break;
    default:            analyze_op(node, env);
  }
}

static void analyze_comp_stmt(Node *node, Map *env) {
  if (node->type != AST_COMP_STMT)
    error("internal error");

  for (int i = 0; i < vector_size(node->stmts); i++) {
    analyze_expr(vector_get(node->stmts, i), env);
  }
}

static void analyze_func(Node *node, Map *env) {
  if (node->type != AST_FUNC)
    error("internal error");

  if (vector_size(node->arguments) > 6)
    error("too many arguments");

  long offset = 0;
  for (int i = 0; i < vector_size(node->arguments); i++) {
    Node *arg = vector_get(node->arguments, i);
    if (arg->type != AST_EXPR || arg->expr->type != AST_LVAR)
      error("invalid parameter %s", node2s(arg));

    /* FIXME: calculate offset by size of type */
    offset -= 8;
    switch (arg->expr->ty->ty) {
      case TYINT:
      case TYPTR:
        arg->expr->ty->offset = offset;
    }
    map_push(env, arg->expr->var_name, (void *)arg->expr->ty);
  }

  analyze_comp_stmt(node->body, env);
}

void analyze_toplevel(Vector *toplevels) {
  for (int i = 0; i < vector_size(toplevels); i++) {
    Node *node = vector_get(toplevels, i);
    node->env = map_new();
    if (node->type == AST_FUNC)
      analyze_func(node, node->env);
    else
      error("internal error");

    Vector *keys = map_keys(node->env);
    for (int k = 0; k < vector_size(keys); k++) {
      char *key = vector_get(keys, k);
      Type *ty = map_get(node->env, key);
      printf("# %s: %ld\n", key, ty->offset);
    }
  }
}
