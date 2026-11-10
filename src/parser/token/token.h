#ifndef TOKEN_H
#define TOKEN_H

#include "memory/arena.h"
#include "util/str.h"

typedef enum tokentype {
	TKN_BOLD,
	TKN_ITALIC,
	TKN_CODESPAN,
	TKN_HYPERLINK,
} TokenType;

typedef struct token {
	TokenType type;
	struct token *next;

	Slice text; // for bold, italic, codespan, hyperlink text
	union {
		Slice linkUrl;
	};
} Token;

Token *tokenMake(Arena *arena, TokenType type);

typedef struct tokenlist {
	Token *head;
	Token *tail;
} TokenList;

void tokenAdd(TokenList *list, Token *block);
void tokenPrint(Token *t);

#endif
