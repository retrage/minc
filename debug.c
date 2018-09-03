#include "minc.h"

void error(char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  fprintf(stderr, "error: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(1);
}

char *tok2s(Token *tok) {
  switch (tok->type) {
    case TEOF:          return "TEOF";
    case TNUMBER:       return format("TNUMBER %s", tok->sval);
    case TIDENT:        return format("TIDENT %s", tok->sval);
    case TKEYWORD:      return "TKEYWORD";
    case TPUNCTUATOR:   return format("TPUNCTUATOR %s", tok->sval);
    default:            return "UNK";
  }
}

void dump_tokens(void) {
  Token *tok = tokens;
  while (1) {
    printf("# %s\n", tok2s(tok));
    if (tok->type == TEOF)
      return;
    tok++;
  }
}

static char *ty2s(Type *ty) {
  Buffer *buf = buffer_new();
  Type *tmp = ty;
  while (tmp->ty == TYPTR) {
    buffer_write(buf, '*');
    tmp = tmp->ptrof;
  }
  switch (tmp->ty) {
    case TYINT:     buffer_append(buf, "int", sizeof("int")); break;
    case TYPTR:     buffer_append(buf, "*", sizeof("*"));     break;
    case TYUNK:     buffer_append(buf, "UNK", sizeof("UNK")); break;
  }

  return buffer_body(buf);
}

static char *op2s(int type) {
  switch (type) {
    case OP_LT:     return "<";
    case OP_GT:     return ">";
    case OP_LE:     return "<=";
    case OP_GE:     return ">=";
    case OP_EQ:     return "==";
    case OP_NEQ:    return "!=";
    case OP_AND:    return "&";
    case OP_XOR:    return "^";
    case OP_OR:     return "|";
    case OP_LOG_AND:return "&&";
    case OP_LOG_OR: return "||";
    case OP_ASSGIN: return "=";
    case OP_ADD:    return "+";
    case OP_SUB:    return "-";
    case OP_MUL:    return "*";
    case OP_DIV:    return "/";
    case OP_REM:    return "%";
    case OP_SHL:    return "<<";
    case OP_SHR:    return ">>";
    default:        return "UNK";
  }
}

static char *vec2s(Vector *vec) {
  if (vector_size(vec) == 0)
    return "";

  Buffer *res = buffer_new();

  for (int i = 0; i < vector_size(vec); i++) {
    if (i > 0)
      buffer_write(res, ' ');

    char *string = node2s(vector_get(vec, i));
    buffer_append(res, string, strlen(string));
  }

  return buffer_body(res);
}

char *node2s(Node *node) {
  if (!node)
    return format("NULL");

  switch (node->type) {
    case AST_FUNC:
      return format("(func fn=%s args=%s %s)",
              node->func_name, vec2s(node->arguments), node2s(node->body));
    case AST_FUNC_CALL:
      return format("(call fn=%s args=%s)",
              node->func_name, vec2s(node->arguments));
    case AST_COMP_STMT:
      return format("(stmt %s)", vec2s(node->stmts));
    case AST_LITERAL:
      return format("%d", node->int_value);
    case AST_LVAR:
      return format("(lvar lv=%s ty=%s)", node->var_name, ty2s(node->ty));
    case AST_EXPR:
      return format("(expr %s)", node2s(node->expr));
    case AST_RETURN:
      return format("(return %s)", node2s(node->retval));
    case AST_IF:
      return format("(if cond=%s then=%s else=%s)",
              node2s(node->cond), node2s(node->then), node2s(node->els));
    case AST_WHILE:
      return format("(while cond=%s then=%s)",
              node2s(node->cond), node2s(node->then));
    case AST_DO_WHILE:
      return format("(do then=%s cond=%s)",
              node2s(node->then), node2s(node->cond));
    case AST_FOR:
      return format("(for init=%s cond=%s incdec=%s then=%s)",
              node2s(node->init), node2s(node->cond),
              node2s(node->incdec), node2s(node->then));
    case AST_GOTO:
      return format("(goto label=%s)", node->label);
    case AST_LABEL:
      return format("(label %s)", node->label);
    case AST_DECL:
      return format("(decl declvar=%s)", node2s(node->declvar));
    case AST_ADDR:
      return format("(addr %s)", node2s(node->operand));
    case AST_DEREF:
      return format("(deref %s)", node2s(node->operand));
    case OP_LT:
    case OP_GT:
    case OP_LE:
    case OP_GE:
    case OP_EQ:
    case OP_NEQ:
    case OP_ASSGIN:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_REM:
    case OP_SHL:
    case OP_SHR:
        return format("(%s %s %s)", 
                op2s(node->type), node2s(node->left), node2s(node->right));
    default:
      return "UNK";
  }
}
