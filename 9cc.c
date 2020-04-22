#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { TK_RESERVED, TK_NUM, TK_EOF } TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token* next;
  int val;
  char* str;
  int len;
};

typedef enum {
  ND_EQ,
  ND_NEQ,
  ND_GT,
  ND_GE,
  ND_LT,
  ND_LE,
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node* lhs;
  Node* rhs;
  int val;
};

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
Node* equality();
Node* relational();
Node* mul();
Node* primary();
Node* unary();

Node* expr() {
  Node* node = equality();
  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, equality());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, equality());
    } else {
      return node;
    }
  }
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
  Node* node = mul();
  for (;;) {
    if (consume("<")) {
      node = new_node(ND_LT, node, mul());
    } else if (consume("<=")) {
      node = new_node(ND_LE, node, mul());
    } else if (consume(">")) {
      node = new_node(ND_GT, node, mul());
    } else if (consume(">=")) {
      node = new_node(ND_GE, node, mul());
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
    return primary();
  }
  if (consume("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());
  }
  return primary();
}

void expect(char op);
int expect_number();
Node* primary() {
  if (consume("(")) {
    Node* node = expr();
    expect(')');
    return node;
  }
  return new_node_num(expect_number());
}

void gen(Node* node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }
  gen(node->lhs);
  gen(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    default:
      break;
  }
  printf("  push rax\n");
}

Token* token;
char* user_input;

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

Token* tokenize(char* p) {
  Token head;
  head.next = NULL;
  Token* cur = &head;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }
    if (strchr("+-*/()", *p)) {
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
    error("トークナイズできません");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }
  user_input = argv[1];
  token = tokenize(user_input);
  Node* node = expr();
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  gen(node);
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}