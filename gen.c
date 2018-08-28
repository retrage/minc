#include "minc.h"

/*
 * The result of operations are remained in rax registers.
 */

static void emit_if(Node *);
static void emit_while(Node *);
static void emit_for(Node *);
static void emit_func_call(Node *);
static void emit_literal(Node *);
static void emit_lvar(Node *);
static void emit_return(Node *);
static void emit_lvalue(Node *);
static void emit_rvalue(Node *);
static void emit_assgin(Node *);
static void emit_op(Node *);
static void emit_expr(Node *);
static void emit_comp_stmt(Node *);
static void emit_func_prologue(Node *);
static void emit_func_epilogue(Node *);

const char *qregs[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

static int ret_label = 0;
static int label_idx = 0;

static int label(void) { return ++label_idx; }

static void emit_if(Node *node) {
  if (node->type != AST_IF)
    error("internal error");

  emit_expr(node->cond);

  int label_then = label();
  int label_els = label();
  int label_end = label();
  printf("\tcmp $0, %%rax\n");
  printf("\tje .L%d\n", label_els);

  printf(".L%d:\n", label_then);
  emit_expr(node->then);
  printf("\tjmp .L%d\n", label_end);

  printf(".L%d:\n", label_els);
  if (node->els)
    emit_expr(node->els);

  printf(".L%d:\n", label_end);
}

static void emit_while(Node *node) {
  if (node->type != AST_WHILE)
    error("internal error");

  int label_begin = label();
  int label_end = label();

  printf(".L%d:\n", label_begin);

  emit_expr(node->cond);

  printf("\tcmp $0, %%rax\n");
  printf("\tje .L%d\n", label_end);

  emit_expr(node->then);

  printf("\tjmp .L%d\n", label_begin);

  printf(".L%d:\n", label_end);
}

static void emit_for(Node *node) {
  if (node->type != AST_FOR)
    error("internal error");

  int label_begin = label();
  int label_end = label();

  emit_expr(node->init);

  printf(".L%d:\n", label_begin);

  emit_expr(node->cond);

  printf("\tcmp $0, %%rax\n");
  printf("\tje .L%d\n", label_end);

  emit_expr(node->then);
  emit_expr(node->incdec);

  printf("\tjmp .L%d\n", label_begin);

  printf(".L%d:\n", label_end);
}

static void emit_func_call(Node *node) {
  if (node->type != AST_FUNC_CALL)
    error("internal error");

  /* FIXME: Number of arguments must be less than 6 */
  for (int i = 0; i < vector_size(node->arguments); i++) {
    emit_expr(vector_get(node->arguments, i));
    printf("\tmov %%rax, %%%s\n", qregs[i]);
  }

  printf("\tcall %s\n", node->func_name);
}

static void emit_literal(Node *node) {
  if (node->type != AST_LITERAL)
    error("internal error");

  printf("\tmovl $%d, %%eax\n", node->int_value);
}

static void emit_lvar(Node *node) {
  if (node->type != AST_LVAR)
    error("internal error");

  printf("\tleaq %ld(%%rbp), %%rax\n", node->offset);
  printf("\tmovq (%%rax), %%rax\n");
}

static void emit_return(Node *node) {
  if (node->type != AST_RETURN)
    error("internal error");

  emit_rvalue(node->retval);

  printf("\tjmp .L%d\n", ret_label);
}

static void emit_lvalue(Node *node) {
  if (node->type != AST_LVAR)
    error("lvalue must be lvar");

  printf("\tleaq %ld(%%rbp), %%rax\n", node->offset);
}

static void emit_rvalue(Node *node) {
  emit_expr(node);
}

static void emit_assgin(Node *node) {
  if (node->type != OP_ASSGIN)
    error("internal error");

  emit_lvalue(node->left);
  printf("\tpush %%rax\n");
  emit_rvalue(node->right);
  printf("\tpush %%rax\n");

  printf("\tpop %%rdi\n");
  printf("\tpop %%rax\n");

  printf("\tmovl %%edi, (%%rax)\n");
}

static void emit_op(Node *node) {
  emit_expr(node->left);
  printf("\tpush %%rax\n");
  emit_expr(node->right);
  printf("\tpush %%rax\n");

  printf("\tpop %%rdi\n");
  printf("\tpop %%rax\n");
  
  switch (node->type) {
    case OP_EQ:
      printf("\tcmpl %%eax, %%edi\n");
      printf("\tsete %%al\n");
      printf("\tmovzbl %%al, %%eax\n");
      break;
    case OP_NEQ:
      printf("\tcmpl %%eax, %%edi\n");
      printf("\tsetne %%al\n");
      printf("\tmovzbl %%al, %%eax\n");
      break;
    case OP_ASSGIN:
      printf("\tmovl %%edi, (%%rax)\n");
      break;
    case OP_ADD:
      printf("\taddl %%edi, %%eax\n");
      break;
    case OP_SUB:
      printf("\tsubl %%edi, %%eax\n");
      break;
    case OP_MUL:
      printf("\tmul %%edi\n");
      break;
    case OP_DIV:
      printf("\tcqto\n");
      printf("\tdiv %%edi\n");
      break;
    default:
      error("internal error");
  }
}

static void emit_expr(Node *node) {
  if (node->type == AST_EXPR) {
    emit_expr(node->expr);
    return;
  }

  switch (node->type) {
    case AST_FUNC_CALL: emit_func_call(node); break;
    case AST_COMP_STMT: emit_comp_stmt(node); break;
    case AST_LITERAL:   emit_literal(node);   break;
    case AST_LVAR:      emit_lvar(node);      break;
    case AST_EXPR:      emit_expr(node);      break;
    case AST_RETURN:    emit_return(node);    break;
    case AST_IF:        emit_if(node);        break;
    case AST_WHILE:     emit_while(node);     break;
    case AST_FOR:       emit_for(node);       break;
    case OP_EQ:
    case OP_NEQ:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:        emit_op(node);        break;
    case OP_ASSGIN:     emit_assgin(node);    break;
    default:            error("internal error");
  }
}

static void emit_comp_stmt(Node *node)  {
  if (node->type != AST_COMP_STMT)
    error("internal error");

  for (int i = 0; i < vector_size(node->stmts); i++)
    emit_expr(vector_get(node->stmts, i));
}

static void emit_func_prologue(Node *node) {
  if (node->type != AST_FUNC)
    error("internal error");

  if (!strcmp(node->func_name, "main"))
    printf(".global main\n");
  printf("%s:\n", node->func_name);
  printf("\tpush %%rbp\n");
  printf("\tmov %%rsp, %%rbp\n");

  ret_label = label();

  long offset;
  offset = 8 * map_size(node->env);
  if (offset > 0)
    printf("\tsub $%ld, %%rsp\n", offset);

  Node *arg;
  for (int i = 0; i < vector_size(node->arguments); i++) {
    arg = vector_get(node->arguments, i);
    offset = (long)map_get(node->env, arg->expr->var_name);
    printf("\tmov %%%s, %ld(%%rbp)\n", qregs[i], offset);
  }
}

static void emit_func_epilogue(Node *node) {
  if (node->type != AST_FUNC)
    error("internal error");

  printf(".L%d:\n", ret_label);

  long offset = 8 * map_size(node->env);
  if (offset > 0)
    printf("\tadd $%ld, %%rsp\n", offset);

  printf("\tpop %%rbp\n");
  printf("\tret\n");
}

void emit_toplevel(Vector *toplevels) {
  for (int i = 0; i < vector_size(toplevels); i++) {
    Node *node = vector_get(toplevels, i);
    if (node->type == AST_TESTVECTOR) {
      printf("main:\n");
      int res = test_vector();
      printf("\tmovl $%d, %%eax\n", res);
    } else if (node->type == AST_TESTMAP) {
      printf("main:\n");
      int res = test_map();
      printf("\tmovl $%d, %%eax\n", res);
    } else if (node->type == AST_FUNC) {
      emit_func_prologue(node);
      emit_comp_stmt(node->body);
      emit_func_epilogue(node);
    }
  }
}
