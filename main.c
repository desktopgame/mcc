#include <stdio.h>

#include "9cc.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "引数の数が正しくありません\n");
    return 1;
  }
  CFile* cfile = cfile_new(argv[1]);
  cfile_lex(cfile);
  cfile_parse(cfile);
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  cfile_generate(cfile);
  return 0;
}