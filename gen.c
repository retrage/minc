#include "minc.h"

/*
 * The result of operations are remained in rax registers.
 */

static void emit_if(Node *);
static void emit_while(Node *);
static void emit_for(Node *);
static void emit_decl(Node *);
static void emit_addr(Node *);
static void emit_deref(Node *);
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

static void emit_decl(Node *node) {
  /* Do nothing */
}

static void emit_addr(Node *node) {
  if (node->type != AST_ADDR)
    error("internal error");

  if (!node->operand)
    error("internal error");

  if (!node->operand->expr)
    error("internal error");

  Node *expr = node->operand->expr;
  int offset = expr->ty->offset;
  printf("\tleaq %d(%%rbp), %%rax\n", -offset);
}

static void emit_deref(Node *node) {
  if (node->type != AST_DEREF)
    error("internal error");

  if (!node->operand)
    error("internal error");

  if (!node->operand->expr)
    error("internal error");

  Node *expr = node->operand->expr;
  int offset = expr->ty->offset;
  printf("\tmovq %d(%%rbp), %%rax\n", -offset);
  printf("\tmov (%%rax), %%rax\n");
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

  int offset = node->ty->offset;
  printf("\tleaq %d(%%rbp), %%rax\n", -offset);
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

  int offset = node->ty->offset;
  printf("\tleaq %d(%%rbp), %%rax\n", -offset);
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
    case OP_LT:
      printf("\tcmpl %%edi, %%eax\n");
      printf("\tsetl %%al\n");;
      printf("\tmovzbl %%al, %%eax\n");
      break;
    case OP_GT:
      printf("\tcmpl %%edi, %%eax\n");
      printf("\tsetg %%al\n");;
      printf("\tmovzbl %%al, %%eax\n");
      break;
    case OP_LE:
      printf("\tcmpl %%edi, %%eax\n");
      printf("\tsetle %%al\n");;
      printf("\tmovzbl %%al, %%eax\n");
      break;
    case OP_GE:
      printf("\tcmpl %%edi, %%eax\n");
      printf("\tsetge %%al\n");;
      printf("\tmovzbl %%al, %%eax\n");
      break;
    case OP_EQ:
      printf("\tcmpl %%edi, %%eax\n");
      printf("\tsete %%al\n");
      printf("\tmovzbl %%al, %%eax\n");
      break;
    case OP_NEQ:
      printf("\tcmpl %%edi, %%eax\n");
      printf("\tsetne %%al\n");
      printf("\tmovzbl %%al, %%eax\n");
      break;
    case OP_AND:
      printf("\tandl %%edi, %%eax\n");
      break;
    case OP_XOR:
      printf("\txorl %%edi, %%eax\n");
      break;
    case OP_OR:
      printf("\torl %%edi, %%eax\n");
      break;
    case OP_LOG_AND:
      {
      int label_end = label();
      printf("\tcmpl $0, %%edi\n");
      printf("\tje .L%d\n", label_end);
      printf("\tcmpl $0, %%eax\n");
      printf("\tje .L%d\n", label_end);
      printf("\tmov1 $1, %%eax\n");
      printf(".L%d:\n", label_end);
      }
      break;
    case OP_LOG_OR:
      {
      int label_end = label();
      printf("\tcmpl $1, %%edi\n");
      printf("\tjne .L%d\n", label_end);
      printf("\tcmpl $1, %%eax\n");
      printf("\tjne .L%d\n", label_end);
      printf("\tmov1 $0, %%eax\n");
      printf(".L%d:\n", label_end);
      }
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
    case OP_REM:
      printf("\tcqto\n");
      printf("\tdiv %%edi\n");
      printf("\tmov %%edx, %%eax\n");
      break;
    case OP_SHL:
      printf("\tmov %%rdi, %%rcx\n");
      printf("\tshl %%cl, %%eax\n");
      break;
    case OP_SHR:
      printf("\tmov %%rdi, %%rcx\n");
      printf("\tsar %%cl, %%eax\n");
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
    case AST_DECL:      emit_decl(node);      break;
    case AST_ADDR:      emit_addr(node);      break;
    case AST_DEREF:     emit_deref(node);     break;
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
    case OP_SHR:        emit_op(node);        break;
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

  int offset = calc_offset(node->env);
  if (offset > 0)
    printf("\tsub $%d, %%rsp\n", offset);

  for (int i = 0; i < vector_size(node->arguments); i++) {
    Node *arg = vector_get(node->arguments, i);
    Type *ty = map_get(node->env, arg->expr->var_name);
    int offset = ty->offset;
    printf("\tmov %%%s, %d(%%rbp)\n", qregs[i], -offset);
  }
}

static void emit_func_epilogue(Node *node) {
  if (node->type != AST_FUNC)
    error("internal error");

  printf(".L%d:\n", ret_label);

  int offset = calc_offset(node->env);
  if (offset > 0)
    printf("\tadd $%d, %%rsp\n", offset);

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
