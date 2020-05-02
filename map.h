#ifndef MAP_H
#define MAP_H
#include <stdbool.h>

// 二分探索木
typedef struct Map Map;

struct Map {
  char* key;
  void* value;
  Map* left;
  Map* right;
};

void map_init(Map* self, const char* key, void* value);

Map* map_new();

void map_set(Map* self, const char* key, void* value);

bool map_get(Map* self, const char* key, void** outValue);
#endif