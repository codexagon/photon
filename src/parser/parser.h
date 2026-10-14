#ifndef PARSER_H
#define PARSER_H

#include "memory/arena.h"
#include "util/slice.h"

#define METADATA_CAP 4

typedef struct metadata {
	Slice keys[METADATA_CAP];
	Slice values[METADATA_CAP];
	int len;
} Metadata;

void metadataPrint(Metadata *m);

Metadata *parseMetadata(Arena *arena, const char *file);

#endif
