#include <stdlib.h>
#include <string.h>

#include "9cc.h"

void map_init(Map* self, const char* key) {
  self->key = strdup(key);
  self->value = NULL;
  self->left = NULL;
  self->right = NULL;
}

Map* map_new() {
  Map* m = malloc(sizeof(Map));
  map_init(m, "\0");
  return m;
}

void map_set(Map* self, const char* key, void* value) {
  int c = strcmp(self->key, key);
  if (c == 0) {
    self->value = value;
  } else if (c < 0) {
    if (!self->left) {
      self->left = map_new();
      self->left->key = strdup(key);
      self->left->value = value;
    } else {
      map_set(self->left, key, value);
    }
  } else if (c > 0) {
    if (!self->right) {
      self->right = map_new();
      self->right->key = strdup(key);
      self->right->value = value;
    } else {
      map_set(self->right, key, value);
    }
  }
}

bool map_get(Map* self, const char* key, void** outValue) {
  int c = strcmp(self->key, key);
  if (c == 0) {
    (*outValue) = self->value;
    return true;
  } else if (c < 0) {
    if (!self->left) goto fail;
    return map_get(self->left, key, outValue);
  } else if (c > 0) {
    if (!self->right) goto fail;
    return map_get(self->right, key, outValue);
  }
fail : {
  (*outValue) = NULL;
  return false;
}
}