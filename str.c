#include "str.h"

#include <stdlib.h>

char* str_range(char* buf, const char* ptr, int len) {
  memset(buf, '\0', len + 1);
  memcpy(buf, ptr, len);
  return buf;
}

char* str_dup(const char* src) {
  int len = strlen(src);
  char* ret = malloc(sizeof(char) * len);
  memcpy(ret, src, len);
  return ret;
}