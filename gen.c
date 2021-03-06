#include "minc.h"

/*
 * The result of operations are remained in rax registers.
 */

static void emit_if(Node *);
static void emit_while(Node *);
static void emit_do_while(Node *);
static void emit_for(Node *);
static void emit_goto(Node *);
static void emit_label(Node *);
static void emit_break(Node *);
static void emit_continue(Node *);
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
static void emit_unary(Node *);
static void emit_op(Node *);
static void emit_expr(Node *);
static void emit_comp_stmt(Node *);
static void emit_func_prologue(Node *);
static void emit_func_epilogue(Node *);

char *lregs[] = { "edi", "esi", "edx", "ecx", "r8d", "r9d" };
char *qregs[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

static void emit_if(Node *node) {
  if (node->type != AST_IF)
    error("internal error");

  emit_expr(node->cond);

  printf("\tcmp $0, %%rax\n");
  printf("\tje %s\n", node->els->label);

  printf("%s:\n", node->then->label);
  emit_expr(node->then);
  printf("\tjmp %s\n", node->label);

  printf("%s:\n", node->els->label);
  emit_expr(node->els);

  printf("%s:\n", node->label);
}

static void emit_while(Node *node) {
  if (node->type != AST_WHILE)
    error("internal error");

  printf("%s:\n", node->cond->label);

  emit_expr(node->cond);

  printf("\tcmp $0, %%rax\n");
  printf("\tje %s\n", node->label);

  emit_expr(node->then);

  printf("\tjmp %s\n", node->cond->label);

  printf("%s:\n", node->label);
}

static void emit_do_while(Node *node) {
  if (node->type != AST_DO_WHILE)
    error("internal error");

  printf("%s:\n", node->then->label);

  emit_expr(node->then);

  printf("%s:\n", node->cond->label);

  emit_expr(node->cond);

  printf("\tcmp $0, %%rax\n");
  printf("\tje %s\n", node->label);

  printf("\tjmp %s\n", node->then->label);

  printf("%s:\n", node->label);
}

static void emit_for(Node *node) {
  if (node->type != AST_FOR)
    error("internal error");

  emit_expr(node->init);

  printf("%s:\n", node->cond->label);

  emit_expr(node->cond);

  printf("\tcmp $0, %%rax\n");
  printf("\tje %s\n", node->label);

  emit_expr(node->then);

  printf("%s:\n", node->incdec->label);

  emit_expr(node->incdec);

  printf("\tjmp %s\n", node->cond->label);

  printf("%s:\n", node->label);
}

static void emit_goto(Node *node) {
  if (node->type != AST_GOTO)
    error("internal error");

  printf("\tjmp %s\n", node->dest);
}

static void emit_label(Node *node) {
  if (node->type != AST_LABEL)
    error("internal error");

  printf("%s:\n", node->label);
}

static void emit_break(Node *node) {
  if (node->type != AST_BREAK)
    error("internal error");

  printf("\tjmp %s\n", node->dest);
}

static void emit_continue(Node *node) {
  if (node->type != AST_CONTINUE)
    error("internal error");

  printf("\tjmp %s\n", node->dest);
}

static void emit_decl(Node *node) {
  if (node->type != AST_DECL)
    error("internal error");

  Node *expr = node->declvar->expr;
  if (expr->type == OP_ASSGIN)
    emit_expr(expr);
}

static void emit_addr(Node *node) {
  if (node->type != AST_ADDR)
    error("internal error");

  if (!node->operand)
    error("internal error");

  if (!node->operand->expr)
    error("internal error");

  emit_expr(node->operand);

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

  emit_expr(node->operand);

  Node *expr = node->operand->expr;
  int offset = expr->ty->offset;
  printf("\tmovq %d(%%rbp), %%rax\n", -offset);
  printf("\tmov (%%rax), %%rax\n");
}

static void emit_func_call(Node *node) {
  if (node->type != AST_FUNC_CALL)
    error("internal error");

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

  printf("\tjmp %s\n", node->retlabel);
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

  /* FIXME: analyze the type of right operand */
  if (node->right->type == AST_ADDR)
    printf("\tmov %%rdi, (%%rax)\n");
  else {
    if (node->operand_ty && node->operand_ty->ty == TYPTR)
      printf("\tmov %%rdi, (%%rax)\n");
    else
      printf("\tmov %%edi, (%%rax)\n");
  }
}

static void emit_unary(Node *node) {
  if (!node->operand)
    error("internal error");

  emit_expr(node->operand);

  switch (node->type) {
    case AST_ADDR:
      emit_addr(node);
      break;
    case AST_DEREF:
      emit_deref(node);
      break;
    case AST_POS:
      /* do nothing */
      break;
    case AST_NEG:
      printf("\tneg %%eax\n");
      break;
    case AST_COMP:
      printf("\tnot %%eax\n");
      break;
    case AST_LOG_NEG:
      printf("\tcmpl $0, %%eax\n");
      printf("\tsete %%al\n");
      printf("\tmovzbl %%al, %%eax\n");
      break;
    case AST_PRE_INC:
      printf("add $1, %%eax\n");
      break;
    case AST_PRE_DEC:
      printf("sub $1, %%eax\n");
      break;
    default:
      error("internal error");
  }
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
      printf("\tcmpl $0, %%edi\n");
      printf("\tje %s\n", node->label_false);
      printf("\tcmpl $0, %%eax\n");
      printf("\tje %s\n", node->label_false);
      printf("\tmovl $1, %%eax\n");
      printf("\tjmp %s\n", node->label);
      printf("%s:\n", node->label_false);
      printf("\tmovl $0, %%eax\n");
      printf("%s:\n", node->label);
      break;
    case OP_LOG_OR:
      printf("\tcmpl $0, %%edi\n");
      printf("\tjne %s\n", node->label_true);
      printf("\tcmpl $0, %%eax\n");
      printf("\tje %s\n", node->label_false);
      printf("%s:\n", node->label_true);
      printf("\tmovl $1, %%eax\n");
      printf("\tjmp %s\n", node->label);
      printf("%s:\n", node->label_false);
      printf("\tmovl $0, %%eax\n");
      printf("%s:\n", node->label);
      break;
    case OP_ADD:
      if (node->operand_ty) {
        printf("\tpush %%rax\n");
        printf("\tmov %%rdi, %%rax\n");
        /* FIXME: compute size of type */
        printf("\tmovl $%d, %%edi\n", 4);
        printf("\tmul %%edi\n");
        printf("\tpop %%rdi\n");
        printf("\tadd %%rdi, %%rax\n");
      } else
        printf("\taddl %%edi, %%eax\n");
      break;
    case OP_SUB:
      if (node->operand_ty) {
        printf("\tpush %%rax\n");
        printf("\tmov %%rdi, %%rax\n");
        /* FIXME: compute size of type */
        printf("\tmovl $%d, %%edi\n", 4);
        printf("\tmul %%edi\n");
        printf("\tpush %%rax\n");
        printf("\tmov %%rdi, %%rax\n");
        printf("\tpop %%rdi\n");
        printf("\tpop %%rax\n");
        printf("\tsub %%rdi, %%rax\n");
      } else
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
    case AST_DO_WHILE:  emit_do_while(node);  break;
    case AST_FOR:       emit_for(node);       break;
    case AST_GOTO:      emit_goto(node);      break;
    case AST_LABEL:     emit_label(node);     break;
    case AST_BREAK:     emit_break(node);     break;
    case AST_CONTINUE:  emit_continue(node);  break;
    case AST_DECL:      emit_decl(node);      break;
    case AST_ADDR:
    case AST_DEREF:
    case AST_POS:
    case AST_NEG:
    case AST_COMP:
    case AST_LOG_NEG:
    case AST_PRE_INC:
    case AST_PRE_DEC:   emit_unary(node);     break;
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

  int offset = calc_offset(node->env->lvars);
  if (offset > 0)
    printf("\tsub $%d, %%rsp\n", offset);

  for (int i = 0; i < vector_size(node->arguments); i++) {
    Node *arg = vector_get(node->arguments, i);
    Type *ty = map_get(node->env->lvars, arg->expr->var_name);
    int offset = ty->offset;
    char *regs;
    switch (ty->ty) {
      case TYINT:
        regs = lregs[i];
        break;
      case TYPTR:
        regs = qregs[i];
        break;
      case TYUNK:
      default:
        error("internal error");
    }
    printf("\tmov %%%s, %d(%%rbp)\n", regs, -offset);
  }
}

static void emit_func_epilogue(Node *node) {
  if (node->type != AST_FUNC)
    error("internal error");

  printf("%s:\n", node->label);

  int offset = calc_offset(node->env->lvars);
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
