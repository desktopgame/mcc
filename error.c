#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cfile.h"

void error_at(CFile* cfile, char* loc, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int pos = loc - fmt;
  fprintf(stderr, "%s\n", cfile->sourceRef);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}