#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

#include "common.h"
#include "memory/arena.h"
#include "util/str.h"

String *readFile(Arena *arena, char *filepath);
bool createDir(const char *path);

#endif
