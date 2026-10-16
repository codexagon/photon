#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

#include "common.h"
#include "memory/arena.h"

char *readFile(Arena *arena, char *filepath);

#endif
