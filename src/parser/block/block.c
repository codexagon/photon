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

void blockPrint(Block *t) {
	switch (t->type) {
		case BLK_HEADING: {
			printf("[heading %i \"", t->heading.level);
			slicePrint(t->heading.text);
			printf("\"]\n");
			break;
		}
		case BLK_PARAGRAPH: {
			printf("[paragraph \"");
			slicePrint(t->paragraph);
			printf("\"]\n");
			break;
		}
		case BLK_CODE: {
			printf("[code ");
			slicePrint(t->code.lang);
			printf(", \"");
			slicePrint(t->code.text);
			printf("\"]\n");
			break;
		}
		case BLK_BLOCKQUOTE: {
			printf("[blockquote \"");
			for (int i = 0; i < t->bquote.count; i++) {
				slicePrint(t->bquote.lines[i]);
				printf(" ");
			}
			printf("\"]\n");
			break;
		}
		case BLK_UNORDERED_LIST: {
			printf("[ulist \"");
			for (int i = 0; i < t->list.count; i++) {
				printf("- ");
				slicePrint(t->list.items[i]);
				printf("\n");
			}
			printf("\"]\n");
			break;
		}
		case BLK_ORDERED_LIST: {
			printf("[olist \"");
			for (int i = 0; i < t->list.count; i++) {
				printf("%i. ", i + 1);
				slicePrint(t->list.items[i]);
				printf("\n");
			}
			printf("\"]\n");
			break;
		}
	}
}
