#include "minc.h"

static void scan_label(Node *, Env *);
static void analyze_func_call(Node *, Env *);
static void analyze_if(Node *, Env *);
static void analyze_while(Node *, Env *);
static void analyze_do_while(Node *, Env *);
static void analyze_for(Node *, Env *);
static void analyze_goto(Node *, Env *);
static void analyze_label(Node *, Env *);
static void analyze_break(Node *, Env *);
static void analyze_decl(Node *, Env *);
static void analyze_addr(Node *, Env *);
static void analyze_deref(Node *, Env *);
static void analyze_literal(Node *, Env *);
static void analyze_lvar(Node *, Env *);
static void analyze_op(Node *, Env *);
static void analyze_expr(Node *, Env *);
static void analyze_comp_stmt(Node *, Env *);
static void analyze_func(Node *, Env *);

static int label_idx = 0;
static char *retlabel;
static Vector *controls;

static void add_label(Node *node) {
  node->label = format(".L%d", ++label_idx);
}

static void scan_label(Node *node, Env *env) {
  switch (node->type) {
    case AST_FUNC:
      scan_label(node->body, env);
      break;
    case AST_COMP_STMT:
      for (int i = 0; i < vector_size(node->stmts); i++)
        scan_label(vector_get(node->stmts, i), env);
      break;
    case AST_EXPR:
      scan_label(node->expr, env);
      break;
    case AST_IF:
      scan_label(node->then, env);
      if (node->els)
        scan_label(node->els, env);
      break;
    case AST_WHILE:
    case AST_DO_WHILE:
    case AST_FOR:
      scan_label(node->then, env);
      break;
    case AST_LABEL:
      add_label(node);
      map_push(env->labels, node->str_value, node);
      break;
  }
}

static void analyze_func_call(Node *node, Env *env) {
  if (node->type != AST_FUNC_CALL)
    error("internal error");

  if (vector_size(node->arguments) > 6)
    error("too many arguments");

  for (int i = 0; i < vector_size(node->arguments); i++) {
    Node *arg = vector_get(node->arguments, i);
    analyze_expr(arg, env);
  }
}

static void analyze_if(Node *node, Env *env) {
  if (node->type != AST_IF)
    error("internal error");

  /* XXX: node->label represents end of the statement */
  add_label(node);
  analyze_expr(node->cond, env);
  analyze_comp_stmt(node->then, env);
  add_label(node->then);
  analyze_comp_stmt(node->els, env);
  add_label(node->els);
}

static void analyze_while(Node *node, Env *env) {
  if (node->type != AST_WHILE)
    error("internal error");

  vector_push(controls, node);
  /* XXX: node->label represents end of the statement */
  add_label(node);
  analyze_expr(node->cond, env);
  add_label(node->cond);
  analyze_comp_stmt(node->then, env);
  vector_pop(controls);
}

static void analyze_do_while(Node *node, Env *env) {
  if (node->type != AST_DO_WHILE)
    error("internal error");

  vector_push(controls, node);
  /* XXX: node->label represents end of the statement */
  add_label(node);
  analyze_comp_stmt(node->then, env);
  add_label(node->then);
  analyze_expr(node->cond, env);
  vector_pop(controls);
}

static void analyze_for(Node *node, Env *env) {
  if (node->type != AST_FOR)
    error("internal error");

  vector_push(controls, node);
  /* XXX: node->label represents end of the statement */
  add_label(node);
  analyze_expr(node->init, env);
  analyze_expr(node->cond, env);
  add_label(node->cond);
  analyze_expr(node->incdec, env);
  analyze_comp_stmt(node->then, env);
  vector_pop(controls);
}

static void analyze_goto(Node *node, Env *env) {
  if (node->type != AST_GOTO)
    error("internal error");

  Vector *keys = map_keys(env->labels);
  for (int i = 0; i < vector_size(keys); i++) {
    char *key = vector_get(keys, i);
    if (!strcmp(node->dest, key)) {
      /* XXX: overwrite node->dest */
      Node *label = map_get(env->labels, key);
      node->dest = label->label;
      return;
    }
  }
  error("label '%s' used but not defined", node->dest);
}

static void analyze_label(Node *node, Env *env) {
  if (node->type != AST_LABEL)
    error("internal error");

  Vector *keys = map_keys(env->labels);
  for (int i = 0; i < vector_size(keys); i++) {
    char *key = vector_get(keys, i);
    if (!strcmp(node->str_value, key))
      return;
  }
  error("internal error");
}

static void analyze_break(Node *node, Env *env) {
  if (node->type != AST_BREAK)
    error("internal error");

  Node *ctrl = vector_get(controls, vector_size(controls) - 1);
  node->dest = ctrl->label;
}

static void analyze_decl(Node *node, Env *env) {
  if (node->type != AST_DECL)
    error("internal error");

  Vector *lvars = map_keys(env->lvars);
  Node *declvar;

  Node *expr = node->declvar->expr;
  if (expr->type == OP_ASSGIN) {
    if (expr->left->type != AST_LVAR)
      error("internal error");
    declvar = expr->left;
  } else if (expr->type == AST_LVAR) {
    declvar = expr;
  } else
    error("internal error");

  for (int i = 0; i < vector_size(lvars); i++) {
    if (!strcmp(declvar->var_name, vector_get(lvars, i)))
      error("redeclaration of %s", declvar->var_name);
  }

  int size;
  int offset = calc_offset(env->lvars);
  switch (declvar->ty->ty) {
    case TYINT:
      size = 4;
      break;
    case TYPTR:
      size = 8;
      break;
    case TYUNK:
    default:
      error("internal error");
  }

  declvar->ty->size = size;
  declvar->ty->offset = offset + size;
  map_push(env->lvars, declvar->var_name, (void *)declvar->ty);

  if (expr->type == OP_ASSGIN)
    analyze_expr(expr->right, env);
}

static void analyze_addr(Node *node, Env *env) {
  if (node->type != AST_ADDR)
    error("internal error");

  Node *expr = node->operand->expr;
  if (expr->type != AST_LVAR)
    error("lvalue required as unary '&' operand");

  Type *ty = map_get(env->lvars, expr->var_name);
  if (ty)
    expr->ty = ty;
  else
    error("%s undeclared", expr->var_name);
}

static void analyze_deref(Node *node, Env *env) {
  if (node->type != AST_DEREF)
    error("internal error");

  /* FIXME: check if operand represents address */
  Node *expr = node->operand->expr;
  Type *ty = map_get(env->lvars, expr->var_name);
  if (ty)
    expr->ty = ty;
  else
    error("%s undeclared", expr->var_name);
}

static void analyze_literal(Node *node, Env *env) {
  if (node->type != AST_LITERAL)
    error("internal error");
}

static void analyze_lvar(Node *node, Env *env) {
  if (node->type != AST_LVAR)
    error("internal error");

  Type *ty = map_get(env->lvars, node->var_name);
  if (ty)
    node->ty = ty;
  else
    error("%s undeclared", node->var_name);
}

static void analyze_return(Node *node, Env *env) {
  if (node->type != AST_RETURN)
    error("internal error");

  analyze_expr(node->retval, env);
  node->retlabel = retlabel;
}

static void analyze_op(Node *node, Env *env) {
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
      if (node->type == OP_LOG_AND || node->type == OP_LOG_OR)
        add_label(node);
      analyze_expr(node->left, env);
      analyze_expr(node->right, env);
      break;
    default:
      error("internal error %s", node2s(node));
  }
}

static void analyze_expr(Node *node, Env *env) {
  switch (node->type) {
    case AST_FUNC_CALL: analyze_func_call(node, env); break;
    case AST_COMP_STMT: analyze_comp_stmt(node, env); break;
    case AST_LITERAL:   analyze_literal(node, env);   break;
    case AST_LVAR:      analyze_lvar(node, env);      break;
    case AST_EXPR:      analyze_expr(node->expr, env);break;
    case AST_RETURN:    analyze_return(node, env);    break;
    case AST_IF:        analyze_if(node, env);        break;
    case AST_WHILE:     analyze_while(node, env);     break;
    case AST_DO_WHILE:  analyze_do_while(node, env);  break;
    case AST_FOR:       analyze_for(node, env);       break;
    case AST_GOTO:      analyze_goto(node, env);      break;
    case AST_LABEL:     analyze_label(node, env);     break;
    case AST_BREAK:     analyze_break(node, env);     break;
    case AST_DECL:      analyze_decl(node, env);      break;
    case AST_ADDR:      analyze_addr(node, env);      break;
    case AST_DEREF:     analyze_deref(node, env);     break;
    default:            analyze_op(node, env);
  }
}

static void analyze_comp_stmt(Node *node, Env *env) {
  if (node->type != AST_COMP_STMT)
    error("internal error");

  for (int i = 0; i < vector_size(node->stmts); i++) {
    analyze_expr(vector_get(node->stmts, i), env);
  }
}

static void analyze_func(Node *node, Env *env) {
  if (node->type != AST_FUNC)
    error("internal error");

  if (vector_size(node->arguments) > 6)
    error("too many arguments");

  for (int i = 0; i < vector_size(node->arguments); i++) {
    Node *arg = vector_get(node->arguments, i);
    if (arg->type != AST_EXPR || arg->expr->type != AST_LVAR)
      error("invalid parameter %s", node2s(arg));

    int size;
    int offset = calc_offset(env->lvars);
    switch (arg->expr->ty->ty) {
      case TYINT:
        size = 4;
        break;
      case TYPTR:
        size = 8;
        break;
      default:
        error("internal error");
    }
    arg->expr->ty->size = size;
    arg->expr->ty->offset = offset + size;
    map_push(env->lvars, arg->expr->var_name, (void *)arg->expr->ty);
  }

  /* XXX: node->label represents end of the statement */
  add_label(node);
  retlabel = node->label;
  analyze_comp_stmt(node->body, env);
}

void analyze_toplevel(Vector *toplevels) {
  controls = vector_new();
  for (int i = 0; i < vector_size(toplevels); i++) {
    Node *node = vector_get(toplevels, i);
    if (node->type == AST_FUNC) {
      node->env = malloc(sizeof(Env));
      node->env->lvars = map_new();
      node->env->labels = map_new();

      scan_label(node, node->env);
      analyze_func(node, node->env);
    } else
      error("internal error");

    Vector *keys = map_keys(node->env->lvars);
    for (int k = 0; k < vector_size(keys); k++) {
      char *key = vector_get(keys, k);
      Type *ty = map_get(node->env->lvars, key);
      printf("# %s: %d %d\n", key, ty->size, ty->offset);
    }
  }
}
