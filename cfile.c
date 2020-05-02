#include "cfile.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "error.h"
#include "lvar.h"
#include "node.h"
#include "token.h"

static bool consume(CFile* cfile, char* op);
static Token* consume_ident(CFile* cfile);
static void expect(CFile* cfile, char op);
static int expect_number(CFile* cfile);
static bool at_eof(CFile* cfile);
static bool has_prefix(const char* src, const char* target);
static int is_alnum(char c);
static Token* tokenize(char* p);
static void program(CFile* cfile);
static Node* function(CFile* cfile);
static Node* stmt(CFile* cfile);
static Node* expr(CFile* cfile);
static Node* assign(CFile* cfile);
static Node* equality(CFile* cfile);
static Node* relational(CFile* cfile);
static Node* add(CFile* cfile);
static Node* mul(CFile* cfile);
static Node* unary(CFile* cfile);
static Node* primary(CFile* cfile);

CFile* cfile_new(char* sourceRef) {
  CFile* ret = malloc(sizeof(CFile));
  ret->sourceRef = sourceRef;
  ret->token = NULL;
  ret->nodes = vec_new();
  ret->_defFuncNode = NULL;
  return ret;
}

void cfile_lex(CFile* self) { self->token = tokenize(self->sourceRef); }

void cfile_parse(CFile* self) { program(self); }

void cfile_generate(CFile* self) {
  for (int i = 0; i < self->nodes->size; i++) {
    gen((Node*)vec_at(self->nodes, i));
  }
}

void cfile_free(CFile* self) { free(self); }

// private

// Token
static bool consume(CFile* cfile, char* op) {
  if (cfile->token->kind != TK_RESERVED || strlen(op) != cfile->token->len ||
      memcmp(cfile->token->str, op, cfile->token->len)) {
    return false;
  }
  cfile->token = cfile->token->next;
  return true;
}

static Token* consume_ident(CFile* cfile) {
  if (cfile->token->kind == TK_IDENT) {
    Token* tmp = cfile->token;
    cfile->token = cfile->token->next;
    return tmp;
  }
  return NULL;
}

static bool consume_kind(CFile* cfile, TokenKind k) {
  if (cfile->token->kind == k) {
    Token* tmp = cfile->token;
    cfile->token = cfile->token->next;
    return true;
  }
  return false;
}

static bool consume_return(CFile* cfile) {
  return consume_kind(cfile, TK_RETURN);
}

static void expect(CFile* cfile, char op) {
  if (cfile->token->kind != TK_RESERVED || cfile->token->str[0] != op) {
    error_at(cfile, cfile->token->str, "%cではありません", op);
  }
  cfile->token = cfile->token->next;
}

static int expect_number(CFile* cfile) {
  if (cfile->token->kind != TK_NUM) {
    error_at(cfile, cfile->token->str, "数ではありません");
  }
  int val = cfile->token->val;
  cfile->token = cfile->token->next;
  return val;
}

static bool at_eof(CFile* cfile) { return cfile->token->kind == TK_EOF; }

static bool has_prefix(const char* src, const char* target) {
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

static Token* tokenize(char* p) {
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
    if (strchr(";=+-*/()<>{},", *p)) {
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
      while (('a' <= *p && *p <= 'z') || isdigit(*p) || *p == '_') {
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

static void program(CFile* cfile) {
  int i = 0;
  while (!at_eof(cfile)) {
    vec_push(cfile->nodes, function(cfile));
  }
}

static Node* function(CFile* cfile) {
  DefFuncNode* defFuncNode = calloc(1, sizeof(DefFuncNode));
  defFuncNode->locals = NULL;
  cfile->_defFuncNode = defFuncNode;
  Node* node = (Node*)defFuncNode;
  Node* retAndParamsNode = calloc(1, sizeof(Node));
  Node* nameAndBodyNode = calloc(1, sizeof(Node));
  ReturnTypeNode* retNode = calloc(1, sizeof(ReturnTypeNode));
  FunctionNameNode* nameNode = calloc(1, sizeof(FunctionNameNode));
  Node* paramsNode = calloc(1, sizeof(Node));
  Node* bodyNode = calloc(1, sizeof(Node));
  node->kind = ND_DEFFUN;
  node->lhs = retAndParamsNode;
  node->rhs = nameAndBodyNode;
  retAndParamsNode->lhs = (Node*)retNode;
  retAndParamsNode->rhs = paramsNode;
  nameAndBodyNode->lhs = (Node*)nameNode;
  nameAndBodyNode->rhs = bodyNode;
  Token* ret = consume_ident(cfile);
  retNode->name = ret->str;
  retNode->len = ret->len;
  if (!ret) {
    error("トークンは識別子ではありません");
  }
  Token* name = consume_ident(cfile);
  nameNode->name = name->str;
  nameNode->len = name->len;
  if (!ret) {
    error("トークンは識別子ではありません");
  }
  if (!consume(cfile, "(")) {
    error("関数の引数列は ( で開始する必要があります");
  }
  Node* paramsWrite = paramsNode;
  while (!consume(cfile, ")")) {
    Token* param = consume_ident(cfile);
    if (param) {
      ParameterNode* paramNode = calloc(1, sizeof(ParameterNode));
      if (!paramsWrite->lhs) {
        paramsWrite->lhs = (Node*)paramNode;
      } else {
        Node* child = calloc(1, sizeof(Node));
        child->lhs = (Node*)paramNode;
        paramsWrite->rhs = child;
        paramsWrite = child;
      }
    } else {
      consume(cfile, ",");
    }
  }
  if (!consume(cfile, "{")) {
    error("関数の本体は { で開始する必要があります");
  }
  Node* stmtWrite = bodyNode;
  while (!consume(cfile, "}")) {
    Node* stmtNode = stmt(cfile);
    if (!stmtWrite->lhs) {
      stmtWrite->lhs = stmtNode;
    } else {
      Node* child = calloc(1, sizeof(Node));
      child->lhs = stmtNode;
      stmtWrite->rhs = child;
      stmtWrite = child;
    }
  }
  return node;
}

static Node* stmt(CFile* cfile) {
  Node* node;
  if (consume_return(cfile)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr(cfile);
  } else if (consume_kind(cfile, TK_IF)) {
    expect(cfile, '(');
    Node* cond = expr(cfile);
    expect(cfile, ')');
    Node* body = stmt(cfile);
    if (consume_kind(cfile, TK_ELSE)) {
      Node* elseBody = stmt(cfile);
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
  } else if (consume_kind(cfile, TK_WHILE)) {
    expect(cfile, '(');
    Node* cond = expr(cfile);
    expect(cfile, ')');
    Node* body = stmt(cfile);
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    node->lhs = cond;
    node->rhs = body;
    return node;
  } else if (consume_kind(cfile, TK_FOR)) {
    expect(cfile, '(');
    Node* init = NULL;
    Node* cond = NULL;
    Node* update = NULL;
    if (!consume(cfile, ";")) {
      init = expr(cfile);
      expect(cfile, ';');
    }
    if (!consume(cfile, ";")) {
      cond = expr(cfile);
      expect(cfile, ';');
    }
    if (!consume(cfile, ")")) {
      update = expr(cfile);
      expect(cfile, ')');
    }
    Node* body = stmt(cfile);
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
  } else if (consume(cfile, "{")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    Node* write = node;
    while (!consume(cfile, "}")) {
      Node* line = stmt(cfile);
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
    node = expr(cfile);
  }
  if (!consume(cfile, ";")) {
    error_at(cfile, cfile->token->str, "';'ではないトークンです。");
  }
  return node;
}

static Node* expr(CFile* cfile) { return assign(cfile); }

static Node* assign(CFile* cfile) {
  Node* node = equality(cfile);
  if (consume(cfile, "=")) {
    node = new_node(ND_ASSIGN, node, assign(cfile));
  }
  return node;
}

static Node* equality(CFile* cfile) {
  Node* node = relational(cfile);
  for (;;) {
    if (consume(cfile, "==")) {
      node = new_node(ND_EQ, node, relational(cfile));
    } else if (consume(cfile, "!=")) {
      node = new_node(ND_NEQ, node, relational(cfile));
    } else {
      return node;
    }
  }
}

static Node* relational(CFile* cfile) {
  Node* node = add(cfile);
  for (;;) {
    if (consume(cfile, "<")) {
      node = new_node(ND_LT, node, add(cfile));
    } else if (consume(cfile, "<=")) {
      node = new_node(ND_LE, node, add(cfile));
    } else if (consume(cfile, ">")) {
      node = new_node(ND_LT, add(cfile), node);
    } else if (consume(cfile, ">=")) {
      node = new_node(ND_LE, add(cfile), node);
    } else {
      return node;
    }
  }
}

static Node* add(CFile* cfile) {
  Node* node = mul(cfile);
  for (;;) {
    if (consume(cfile, "+")) {
      node = new_node(ND_ADD, node, mul(cfile));
    } else if (consume(cfile, "-")) {
      node = new_node(ND_SUB, node, mul(cfile));
    } else {
      return node;
    }
  }
}

static Node* mul(CFile* cfile) {
  Node* node = unary(cfile);
  for (;;) {
    if (consume(cfile, "*")) {
      node = new_node(ND_MUL, node, unary(cfile));
    } else if (consume(cfile, "/")) {
      node = new_node(ND_DIV, node, unary(cfile));
    } else {
      return node;
    }
  }
}

static Node* unary(CFile* cfile) {
  if (consume(cfile, "+")) {
    return unary(cfile);
  }
  if (consume(cfile, "-")) {
    return new_node(ND_SUB, new_node_num(0), unary(cfile));
  }
  return primary(cfile);
}

static LVar* find_lvar(DefFuncNode* defFuncNode, Token* tok) {
  for (LVar* var = defFuncNode->locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

static Node* primary(CFile* cfile) {
  if (consume(cfile, "(")) {
    Node* node = expr(cfile);
    expect(cfile, ')');
    return node;
  }
  Token* tok = consume_ident(cfile);
  if (tok && consume(cfile, "(")) {
    CallNode* cNode = calloc(1, sizeof(CallNode));
    cNode->name = tok->str;
    cNode->len = tok->len;
    Node* node = (Node*)cNode;
    Node* write = node;
    node->kind = ND_CALL;
    while (!consume(cfile, ")")) {
      Node* arg = expr(cfile);
      consume(cfile, ",");
      if (!write->lhs) {
        write->lhs = arg;
      } else {
        Node* child = calloc(1, sizeof(Node));
        child->kind = ND_CALL;
        child->lhs = arg;
        write->rhs = child;
        write = child;
      }
    }
    return node;
  }
  if (tok) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    LVar* lvar = find_lvar(cfile->_defFuncNode, tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = cfile->_defFuncNode->locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = cfile->_defFuncNode->locals == NULL
                         ? 8
                         : cfile->_defFuncNode->locals->offset + 8;
      node->offset = lvar->offset;
      cfile->_defFuncNode->locals = lvar;
    }
    return node;
  }
  return new_node_num(expect_number(cfile));
}