#ifndef RENDER_H
#define RENDER_H

#include "memory/arena.h"
#include "parser/token.h"

void renderBlock(Token *token, char *output);
char *mdToHTML(Arena *arena, TokenList *tlist);

#endif
