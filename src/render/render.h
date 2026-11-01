#ifndef RENDER_H
#define RENDER_H

#include "memory/arena.h"
#include "parser/block/block.h"

int renderBlock(Arena *arena, Block *block, String *output);

String *mdToHTML(Arena *arena, BlockList *blist);

#endif
