#pragma once
#include <stdbool.h>
// Token
typedef enum {
  TK_RESERVED,
  TK_IDENT,
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_FOR,
  TK_RETURN,
  TK_NUM,
  TK_EOF
} TokenKind;

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
  ND_CALL,
  ND_DEFFUN,
  ND_BLOCK,
  ND_IF,
  ND_ELSE,
  ND_WHILE,
  ND_FOR,
  ND_RETURN,
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

typedef struct CallNode CallNode;
struct CallNode {
  Node node;
  char* name;
  int len;
};
typedef CallNode ReturnTypeNode;
typedef CallNode ParameterNode;
typedef CallNode FunctionNameNode;
extern Node* code[100];
Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
void program();
Node* function();
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
typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar* next;  // 次の変数かNULL
  char* name;  // 変数の名前
  int len;     // 名前の長さ
  int offset;  // RBPからのオフセット
};

// ローカル変数
extern LVar* locals;
void gen(Node* node);

// 可変長配列
typedef struct Vec Vec;

struct Vec {
  void** data;
  int capa;
  int size;
};

void vec_init(Vec* self);

void vec_push(Vec* self, void* a);

void* vec_at(Vec* self, int index);

void vec_assign(Vec* self, int index, void* a);

// 二分探索木
typedef struct Map Map;

struct Map {
  char* key;
  void* value;
  Map* left;
  Map* right;
};

void map_init(Map* self, const char* key);

Map* map_new();

void map_set(Map* self, const char* key, void* value);

bool map_get(Map* self, const char* key, void** outValue);