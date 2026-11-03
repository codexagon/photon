#ifndef TOKEN_H
#define TOKEN_H

#include "memory/arena.h"
#include "util/str.h"

typedef enum tokentype {
	TKN_BOLD,
	TKN_ITALIC,
	TKN_CODESPAN,
} TokenType;

typedef struct token {
	TokenType type;
	struct token *next;

	Slice text; // for bold, italic, codespan text
} Token;

Token *tokenMake(Arena *arena, TokenType type);

typedef struct tokenlist {
	Token *head;
	Token *tail;
} TokenList;

void tokenAdd(TokenList *list, Token *block);
void tokenPrint(Token *t);

#endif
