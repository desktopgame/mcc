#ifndef TYPE_H
#define TYPE_H
typedef struct Type Type;

struct Type {
  enum { INT, PTR } ty;
  struct Type* ptr_to;
};

void type_init(Type* type, int ty);

Type* type_new(int ty);

Type* type_ptrtype(Type* type);

void type_free(Type* type);
#endif