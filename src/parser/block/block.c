#include "block.h"
#include "common.h"

Block *blockMake(Arena *arena, BlockType type) {
	Block *t = arenaAlloc(arena, sizeof(Block));
	if (!t) {
		emitError("parser", "couldn't create block");
		return NULL;
	}
	t->type = type;
	t->next = NULL;
	return t;
}

void blockAdd(BlockList *list, Block *block) {
	if (list->head == NULL) {
		list->head = block;
		list->tail = block;
		return;
	}

	list->tail->next = block;
	list->tail = block;
}

void blockPrint(Block *b) {
	switch (b->type) {
		case BLK_HEADING: {
			printf("[heading %i \"", b->heading.level);
			slicePrint(b->text);
			printf("\"]\n");
			break;
		}
		case BLK_PARAGRAPH: {
			printf("[paragraph \"");
			slicePrint(b->text);
			printf("\"]\n");
			break;
		}
		case BLK_CODE: {
			printf("[code ");
			slicePrint(b->code.lang);
			printf(", \"");
			slicePrint(b->text);
			printf("\"]\n");
			break;
		}
		case BLK_BLOCKQUOTE: {
			printf("[blockquote \"");
			for (int i = 0; i < b->collection.count; i++) {
				slicePrint(b->collection.items[i]);
				printf(" ");
			}
			printf("\"]\n");
			break;
		}
		case BLK_UNORDERED_LIST: {
			printf("[ulist \"");
			for (int i = 0; i < b->collection.count; i++) {
				printf("- ");
				slicePrint(b->collection.items[i]);
				printf("\n");
			}
			printf("\"]\n");
			break;
		}
		case BLK_ORDERED_LIST: {
			printf("[olist \"");
			for (int i = 0; i < b->collection.count; i++) {
				printf("%i. ", i + 1);
				slicePrint(b->collection.items[i]);
				printf("\n");
			}
			printf("\"]\n");
			break;
		}
	}
}
