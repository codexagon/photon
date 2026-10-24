#ifndef BLOCK_H
#define BLOCK_H

#include "memory/arena.h"
#include "util/slice.h"

typedef enum blocktype {
	BLK_HEADING,
	BLK_PARAGRAPH,
	BLK_CODE,
	BLK_BLOCKQUOTE,
} BlockType;

typedef struct block {
	BlockType type;
	struct block *next;
	union {
		struct {
			int level;
			Slice text;
		} heading;
		Slice paragraph;
		struct {
			Slice lang;
			Slice text;
		} code;
		struct {
			int count;
			Slice *lines;
		} bquote;
	};
} Block;

Block *blockMake(Arena *arena, BlockType type);

typedef struct blocklist {
	Block *head;
	Block *tail;
} BlockList;

void blockAdd(BlockList *list, Block *block);
void blockPrint(Block *t);

#endif
