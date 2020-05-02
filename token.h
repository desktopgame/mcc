#ifndef TOKEN_H
#define TOKEN_H
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
Token* new_token(TokenKind kind, Token* cur, char* str);
#endif