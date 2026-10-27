#ifndef PARSER_H
#define PARSER_H

#include "block/block.h"
#include "memory/arena.h"
#include "util/slice.h"

// metadata parsing

#define METADATA_CAP 8

typedef struct metadata {
	Slice keys[METADATA_CAP];
	Slice values[METADATA_CAP];
	int len;
} Metadata;

void metadataPrint(Metadata *m);

Metadata *parseMetadata(Arena *arena, char *file);
char *jumpOverMetadata(char *file);

// file "blockization" or whatever it's called
BlockList *makeBlocks(Arena *arena, char *file);

Block *makeHeadingBlock(Arena *arena, char **f);
Block *makeCodeBlock(Arena *arena, char **f);
Block *makeParagraphBlock(Arena *arena, char **f);
Block *makeBlockquoteBlock(Arena *arena, char **f);
Block *makeUnorderedListBlock(Arena *arena, char **f);
Block *makeOrderedListBlock(Arena *arena, char **f);

#endif
