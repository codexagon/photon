#ifndef TOKEN_H
#define TOKEN_H

#include "memory/arena.h"
#include "util/slice.h"

typedef enum tokentype {
	TKN_HEADING,
	TKN_PARAGRAPH,
	TKN_CODE,
} TokenType;

typedef struct token {
	TokenType type;
	struct token *next;
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
	};
} Token;

Token *tokenMake(Arena *arena, TokenType type);

typedef struct tokenlist {
	Token *head;
	Token *tail;
} TokenList;

void tokenAdd(TokenList *list, Token *token);
void tokenPrint(Token *t);

#endif
