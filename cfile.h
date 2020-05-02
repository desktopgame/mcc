#ifndef CFILE_H
#define CFILE_H
#include "vec.h"
struct Token;
struct DefFuncNode;

typedef struct CFile CFile;
struct CFile {
  char* sourceRef;
  struct Token* token;
  Vec* nodes;
  struct DefFuncNode* _defFuncNode;
};

CFile* cfile_new(char* sourceRef);

void cfile_lex(CFile* self);

void cfile_parse(CFile* self);

void cfile_generate(CFile* self);

void cfile_free(CFile* self);

#endif