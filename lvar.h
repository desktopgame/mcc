#ifndef LVAR_H
#define LVAR_H
// Code Gen
typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar* next;  // 次の変数かNULL
  char* name;  // 変数の名前
  int len;     // 名前の長さ
  int offset;  // RBPからのオフセット
};

#endif