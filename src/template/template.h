#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "memory/arena.h"
#include "sitegen/sitegen.h"

size_t handlePlaceholder(Slice placeholder, Arena *arena, Page *page, String *out);

String *fillTemplate(Arena *arena, Page *page);

#endif
