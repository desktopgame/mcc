#ifndef VEC_H
#define VEC_H
// 可変長配列
typedef struct Vec Vec;
typedef void (*VecValueFreeFunc)(void*);

struct Vec {
  void** data;
  int capa;
  int size;
};

void vec_init(Vec* self);

Vec* vec_new();

void vec_push(Vec* self, void* a);

void* vec_at(Vec* self, int index);

void vec_assign(Vec* self, int index, void* a);

void vec_free(Vec* self, VecValueFreeFunc func);
#endif