#ifndef BLOCK_H
#define BLOCK_H

#include "memory/arena.h"
#include "parser/token/token.h"
#include "util/str.h"

typedef enum blocktype {
	BLK_HEADING,
	BLK_PARAGRAPH,
	BLK_CODE,
	BLK_BLOCKQUOTE,
	BLK_UNORDERED_LIST,
	BLK_ORDERED_LIST,
} BlockType;

typedef struct block {
	BlockType type;
	struct block *next;

	Slice text; // for heading, paragraph, code
	union {
		struct {
			int level;
		} heading;
		struct {
			Slice lang;
		} code;
		struct {
			int count;
			Slice *items;
		} collection;
	};
} Block;

Block *blockMake(Arena *arena, BlockType type);

typedef struct blocklist {
	Block *head;
	Block *tail;
} BlockList;

void blockAdd(BlockList *list, Block *block);
void blockPrint(Block *b);

#endif
