#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

Token* token = NULL;
char* user_input = NULL;
// Token
void error_at(char* loc, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int pos = loc - fmt;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(char* op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next;
  return true;
}

Token* consume_ident() {
  if (token->kind == TK_IDENT) {
    Token* tmp = token;
    token = token->next;
    return tmp;
  }
  return NULL;
}

bool consume_kind(TokenKind k) {
  if (token->kind == k) {
    Token* tmp = token;
    token = token->next;
    return true;
  }
  return false;
}

bool consume_return() { return consume_kind(TK_RETURN); }

void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error_at(token->str, "%cではありません", op);
  }
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "数ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

Token* new_token(TokenKind kind, Token* cur, char* str) {
  Token* tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

bool has_prefix(const char* src, const char* target) {
  while (*target != '\0') {
    if (*src != *target) {
      return false;
    }
    src++;
    target++;
  }
  return true;
}

static int is_alnum(char c) { return isalpha(c) || isdigit(c) || c == '_'; }

Token* tokenize(char* p) {
  Token head;
  head.next = NULL;
  Token* cur = &head;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }
    if (has_prefix(p, "==") || has_prefix(p, "!=") || has_prefix(p, "<=") ||
        has_prefix(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }
    if (strchr(";=+-*/()<>{}", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }
    if (isdigit(*p)) {
      char buf[16];
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      sprintf(buf, "%d", cur->val);
      cur->len = strlen(buf);
      continue;
    }
    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p);
      cur->len = 6;
      p += 6;
      continue;
    }
    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TK_IF, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }
    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_ELSE, cur, p);
      cur->len = 4;
      p += 4;
      continue;
    }
    if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      cur = new_token(TK_WHILE, cur, p);
      cur->len = 5;
      p += 5;
      continue;
    }
    if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      cur = new_token(TK_FOR, cur, p);
      cur->len = 3;
      p += 3;
      continue;
    }
    if ('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p++);
      cur->len = 1;
      while (('a' <= *p && *p <= 'z')) {
        p++;
        cur->len++;
      }
      continue;
    }
    error("トークナイズできません: %c", *p);
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}
// Node
Node* code[100];

Node* new_node(NodeKind kind, Node* lhs, Node* rhs) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node* new_node_num(int val) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

bool consume(char* op);

void program() {
  int i = 0;
  while (!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

Node* stmt() {
  Node* node;
  if (consume_return()) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else if (consume_kind(TK_IF)) {
    expect('(');
    Node* cond = expr();
    expect(')');
    Node* body = stmt();
    if (consume_kind(TK_ELSE)) {
      Node* elseBody = stmt();
      node = calloc(1, sizeof(Node));
      Node* elseNode = calloc(1, sizeof(Node));
      elseNode->kind = ND_ELSE;
      elseNode->lhs = body;
      elseNode->rhs = elseBody;
      node->kind = ND_IF;
      node->lhs = cond;
      node->rhs = elseNode;
    } else {
      node = calloc(1, sizeof(Node));
      node->kind = ND_IF;
      node->lhs = cond;
      node->rhs = body;
    }
    return node;
  } else if (consume_kind(TK_WHILE)) {
    expect('(');
    Node* cond = expr();
    expect(')');
    Node* body = stmt();
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    node->lhs = cond;
    node->rhs = body;
    return node;
  } else if (consume_kind(TK_FOR)) {
    expect('(');
    Node* init = NULL;
    Node* cond = NULL;
    Node* update = NULL;
    if (!consume(";")) {
      init = expr();
      expect(';');
    }
    if (!consume(";")) {
      cond = expr();
      expect(';');
    }
    if (!consume(")")) {
      update = expr();
      expect(')');
    }
    Node* body = stmt();
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    Node* updateAndBody = calloc(1, sizeof(Node));
    updateAndBody->kind = ND_FOR;
    updateAndBody->lhs = update;
    updateAndBody->rhs = body;
    Node* condAndUpdateAndBody = calloc(1, sizeof(Node));
    condAndUpdateAndBody->kind = ND_FOR;
    condAndUpdateAndBody->lhs = cond;
    condAndUpdateAndBody->rhs = updateAndBody;
    node->lhs = init;
    node->rhs = condAndUpdateAndBody;
    return node;
  } else if (consume("{")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    Node* write = node;
    while (!consume("}")) {
      Node* line = stmt();
      if (!write->lhs) {
        write->lhs = line;
      } else {
        Node* child = calloc(1, sizeof(Node));
        child->lhs = line;
        child->kind = ND_BLOCK;
        write->rhs = child;
        write = child;
      }
    }
    return node;
  } else {
    node = expr();
  }
  if (!consume(";")) {
    error_at(token->str, "';'ではないトークンです。");
  }
  return node;
}

Node* expr() { return assign(); }

Node* assign() {
  Node* node = equality();
  if (consume("=")) {
    node = new_node(ND_ASSIGN, node, assign());
  }
  return node;
}

Node* equality() {
  Node* node = relational();
  for (;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_node(ND_NEQ, node, relational());
    } else {
      return node;
    }
  }
}

Node* relational() {
  Node* node = add();
  for (;;) {
    if (consume("<")) {
      node = new_node(ND_LT, node, add());
    } else if (consume("<=")) {
      node = new_node(ND_LE, node, add());
    } else if (consume(">")) {
      node = new_node(ND_LT, add(), node);
    } else if (consume(">=")) {
      node = new_node(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

Node* add() {
  Node* node = mul();
  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

Node* mul() {
  Node* node = unary();
  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

Node* unary() {
  if (consume("+")) {
    return unary();
  }
  if (consume("-")) {
    return new_node(ND_SUB, new_node_num(0), unary());
  }
  return primary();
}
static LVar* find_lvar(Token* tok) {
  for (LVar* var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}
Node* primary() {
  if (consume("(")) {
    Node* node = expr();
    expect(')');
    return node;
  }
  Token* tok = consume_ident();
  if (tok) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    LVar* lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals == NULL ? 8 : locals->offset + 8;
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }
  return new_node_num(expect_number());
}