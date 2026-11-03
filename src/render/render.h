#ifndef RENDER_H
#define RENDER_H

#include "memory/arena.h"
#include "parser/block/block.h"

int getRenderedBlockLen(Block *block);
int getRenderedTokenListLen(TokenList *tlist);

int renderBlockContents(Arena *arena, Slice *text, TokenList *tlist, String *output);
int renderBlock(Arena *arena, Block *block, String *output);

String *mdToHTML(Arena *arena, BlockList *blist);

#endif
