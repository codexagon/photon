#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "memory/arena.h"
#include "sitegen/sitegen.h"

int handlePlaceholder(Slice placeholder, Arena *arena, Page *page, char *out);

char *fillTemplate(Arena *arena, Page *page);

#endif
