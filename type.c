#include "type.h"

#include <stdlib.h>

void type_init(Type* type, int ty) {
  type->ptr_to = NULL;
  type->ty = ty;
}

Type* type_new(int ty) {
  Type* t = malloc(sizeof(Type));
  type_init(t, ty);
  return t;
}

Type* type_ptrtype(Type* type) {
  Type* t = type_new(PTR);
  t->ptr_to = type;
  return t;
}

void type_free(Type* type) {
  if (type->ptr_to) {
    type_free(type->ptr_to);
  }
  free(type);
}