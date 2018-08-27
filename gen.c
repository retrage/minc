#include "minc.h"

/*
 * The result of operations are remained in rax registers.
 */

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

char *qregs[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

static void emit_func_call(Node *node) {
  if (node->type != AST_FUNC_CALL)
    error("internal error #1");

  for (int i = 0; i < vector_size(node->arguments); i++) {
    emit_expr(vector_get(node->arguments, i));
    printf("\tmov %%rax, %%%s\n", qregs[i]);
  }

  printf("\tcall %s\n", node->func_name);
}

static void emit_literal(Node *node) {
  if (node->type != AST_LITERAL)
    error("internal error #2");

  printf("\tmovl $%d, %%eax\n", node->int_value);
}

static void emit_lvar(Node *node) {
  if (node->type != AST_LVAR)
    error("internal error #3");

  printf("\tleaq %ld(%%rbp), %%rax\n", node->offset);
  printf("\tmovq (%%rax), %%rax\n");
}

static void emit_return(Node *node) {
  if (node->type != AST_RETURN)
    error("internal error #4");

  emit_rvalue(node->retval);
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
    error("internal error #5.5");

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
      error("internal error #5");
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
    case OP_EQ:
    case OP_NEQ:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:        emit_op(node);          break;
    case OP_ASSGIN:     emit_assgin(node);      break;
    default:            error("internal error #6");
  }
}

static void emit_comp_stmt(Node *node)  {
  if (node->type != AST_COMP_STMT)
    error("internal error #7");

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

  long offset = 8 * map_size(node->env);
  if (offset > 0) {
    printf("\tmovl $%ld, %%edi\n", offset);
    printf("\tsub %%rdi, %%rsp\n");
  }
}

static void emit_func_epilogue(Node *node) {
  if (node->type != AST_FUNC)
    error("internal error");

  long offset = 8 * map_size(node->env);
  if (offset > 0) {
    printf("\tmovl $%ld, %%edi\n", offset);
    printf("\tadd %%rdi, %%rsp\n");
  }

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
