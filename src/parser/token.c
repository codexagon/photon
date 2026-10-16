#include "token.h"
#include "common.h"

Token *tokenMake(Arena *arena, TokenType type) {
	Token *t = arenaAlloc(arena, sizeof(Token));
	if (!t) {
		emitError("parser", "couldn't create token");
		return NULL;
	}
	t->type = type;
	t->next = NULL;
	return t;
}

void tokenAdd(TokenList *list, Token *token) {
	if (list->head == NULL) {
		list->head = token;
		list->tail = token;
		return;
	}

	list->tail->next = token;
	list->tail = token;
}

void tokenPrint(Token *t) {
	switch (t->type) {
		case TKN_HEADING: {
			printf("[heading %i \"", t->heading.level);
			slicePrint(t->heading.text);
			printf("\"]\n");
			break;
		}
		case TKN_PARAGRAPH: {
			printf("[paragraph \"");
			slicePrint(t->paragraph);
			printf("\"]\n");
			break;
		}
		case TKN_CODE: {
			printf("[code ");
			slicePrint(t->code.lang);
			printf(", \"");
			slicePrint(t->code.text);
			printf("\"]\n");
		}
	}
}
