#ifndef RENDER_H
#define RENDER_H

#include "memory/arena.h"
#include "parser/block/block.h"

void renderBlock(Block *block, char *output);
char *mdToHTML(Arena *arena, BlockList *blist);

#endif
