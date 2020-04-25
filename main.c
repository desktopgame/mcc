#include <stdio.h>

#include "9cc.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }
  user_input = argv[1];
  token = tokenize(user_input);
  program();
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  // a~zまでの変数を確保する
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");
  for (int i = 0; code[i]; i++) {
    gen(code[i]);
    printf("  pop rax\n");
  }
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}