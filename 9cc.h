#pragma once
#include <stdbool.h>
// Token
typedef enum { TK_RESERVED, TK_IDENT, TK_NUM, TK_EOF } TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token* next;
  int val;
  char* str;
  int len;
};
extern Token* token;
extern char* user_input;
void error_at(char* loc, const char* fmt, ...);
void error(const char* fmt, ...);
bool consume(char* op);
Token* consume_ident();
void expect(char op);
int expect_number();
bool at_eof();
Token* new_token(TokenKind kind, Token* cur, char* str);
Token* tokenize(char* p);

// Node
typedef enum {
  ND_ASSIGN,
  ND_LVAR,
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
  int offset;
};
extern Node* code[100];
Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
void program();
Node* stmt();
Node* expr();
Node* assign();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* primary();
// Code Gen
void gen(Node* node);