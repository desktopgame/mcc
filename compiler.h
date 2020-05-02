#ifndef COMPILER_H
#define COMPILER_H
#include "vec.h"
typedef struct Compiler Compiler;

struct Compiler {
  Vec* cFiles;
};
#endif