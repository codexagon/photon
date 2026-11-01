#ifndef PARSER_H
#define PARSER_H

#include "block/block.h"
#include "memory/arena.h"
#include "token/token.h"
#include "util/str.h"

// metadata parsing

#define METADATA_CAP 8

typedef struct metadata {
	Slice keys[METADATA_CAP];
	Slice values[METADATA_CAP];
	int len;
} Metadata;

void metadataPrint(Metadata *m);

Metadata *parseMetadata(Arena *arena, String *f);
bool jumpOverMetadata(String *f);

// file "blockization" or whatever it's called
BlockList *makeBlocks(Arena *arena, String *f);

Block *makeHeadingBlock(Arena *arena, String *f);
Block *makeCodeBlock(Arena *arena, String *f);
Block *makeParagraphBlock(Arena *arena, String *f);
Block *makeBlockquoteBlock(Arena *arena, String *f);
Block *makeUnorderedListBlock(Arena *arena, String *f);
Block *makeOrderedListBlock(Arena *arena, String *f);

// overall parsing
bool parseMarkdown(Arena *arena, String *f, Metadata **metadata, BlockList **blist);

#endif
