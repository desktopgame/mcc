#ifndef NODE_H
#define NODE_H
#include "lvar.h"
// Node
typedef enum {
  ND_ADDR,
  ND_DEREF,
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
typedef struct DefFuncNode DefFuncNode;
struct DefFuncNode {
  Node node;
  LVar* locals;
};

typedef CallNode ReturnTypeNode;
typedef CallNode ParameterNode;
typedef CallNode FunctionNameNode;
Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
#endif