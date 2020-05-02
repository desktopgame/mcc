#include "vec.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vec_init(Vec* self) {
  self->capa = 16;
  self->size = 0;
  self->data = malloc(sizeof(void*) * 16);
  memset(self->data, 0, sizeof(void*) * 16);
}

void vec_push(Vec* self, void* a) {
  if (self->size >= self->capa) {
    int newCapa = self->capa * 2;
    void* newData = realloc(self->data, sizeof(void*) * newCapa);
    if (!newData) {
      perror("vec_push");
      exit(1);
    }
    self->data = newData;
    self->capa = newCapa;
  }
  self->data[self->size] = a;
  self->size++;
}

void* vec_at(Vec* self, int index) {
  assert(index >= 0 && index < self->size);
  return self->data[index];
}

void vec_assign(Vec* self, int index, void* a) {
  assert(index >= 0 && index < self->size);
  self->data[index] = a;
}