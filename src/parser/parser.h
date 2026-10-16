#ifndef PARSER_H
#define PARSER_H

#include "memory/arena.h"
#include "token.h"
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

// file tokenization
TokenList *tokenizeBlocks(Arena *arena, char *file);

Token *tokenizeHeading(Arena *arena, char **f);
Token *tokenizeCodeBlock(Arena *arena, char **f);
Token *tokenizeParagraph(Arena *arena, char **f);

#endif
