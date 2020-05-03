#include "map.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"

void map_init(Map* self, const char* key, void* value) {
  self->key = str_dup(key);
  self->value = value;
  self->left = NULL;
  self->right = NULL;
}

Map* map_new() {
  Map* m = malloc(sizeof(Map));
  map_init(m, "\0", NULL);
  return m;
}

void map_set(Map* self, const char* key, void* value) {
  assert(self);
  int c = strcmp(self->key, key);
  if (c == 0) {
    self->value = value;
  } else if (c < 0) {
    if (!self->left) {
      self->left = malloc(sizeof(Map));
      map_init(self->left, key, value);
    } else {
      map_set(self->left, key, value);
    }
  } else if (c > 0) {
    if (!self->right) {
      self->right = malloc(sizeof(Map));
      map_init(self->right, key, value);
    } else {
      map_set(self->right, key, value);
    }
  }
}

bool map_get(Map* self, const char* key, void** outValue) {
  assert(self);
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

void map_free(Map* self, MapValueFreeFunc func) {
  if (self->left) {
    map_free(self->left, func);
  }
  if (self->right) {
    map_free(self->right, func);
  }
  free(self->key);
  if (func && self->value) {
    func(self->value);
  }
}

static int map_count_rec(Map* self, int count) {
  if (self->left) {
    count += map_count_rec(self->left, count) + 1;
  }
  if (self->right) {
    count += map_count_rec(self->right, count) + 1;
  }
  return count;
}

int map_count(Map* self) { return map_count_rec(self, 0); }