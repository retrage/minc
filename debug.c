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

static char *ty2s(int type) {
  switch (type) {
    case OP_EQ:     return "==";
    case OP_NEQ:    return "!=";
    case OP_ASSGIN: return "=";
    case OP_ADD:    return "+";
    case OP_SUB:    return "-";
    case OP_MUL:    return "*";
    case OP_DIV:    return "/";
    default:        return "UNK";
  }
}

static char *vec2s(Vector *vec) {
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
      return format("(lvar lv=%s)", node->var_name);
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
    case AST_FOR:
      return format("(for init=%s cond=%s incdec=%s then=%s)",
              node2s(node->init), node2s(node->cond),
              node2s(node->incdec), node2s(node->then));
    case AST_DECL:
      return format("(decl declvar=%s)", node2s(node->declvar));
    case OP_EQ:
    case OP_NEQ:
    case OP_ASSGIN:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
        return format("(%s %s %s)", 
                ty2s(node->type), node2s(node->left), node2s(node->right));
    default:
      return "UNK";
  }
}
