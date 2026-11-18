#include "common.h"
#include "parser.h"

bool tokenizeSlice(Arena *arena, TokenList *list, Slice *slice) {
	size_t i = 0;
	while (i < slice->len) {
		Token *newToken = NULL;
		bool isError = false; // to determine whether its an actual error or falling back

		switch (slice->start[i]) {
			case '*': {
				if (slice->start[i + 1] == '*') {
					newToken = makeBoldToken(arena, slice, &i, &isError);
				} else {
					newToken = makeItalicToken(arena, slice, &i, &isError);
				}
				break;
			}
			case '`': {
				newToken = makeCodespanToken(arena, slice, &i, &isError);
				break;
			}
			case '[': {
				newToken = makeHyperlinkToken(arena, slice, &i, &isError);
				break;
			}
			default: {
				i++;
			}
		}

		// error out only if its an actual error, not falling back to normal text
		if (!newToken && isError) {
			return false;
		}

		if (newToken) {
			tokenAdd(list, newToken);
		}
	}

	return true;
}

Token *makeBoldToken(Arena *arena, Slice *s, size_t *i, bool *isError) {
	Token *t = tokenMake(arena, TKN_BOLD);
	if (!t) {
		emitError("parser", "couldn't create token");
		*isError = true;
		return NULL;
	}

	size_t initial = *i;

	*i += 2; // skip the opening '**'
	while (*i + 1 < s->len && !(s->start[*i] == '*' && s->start[*i + 1] == '*')) {
		(*i)++;
	}

	if (*i + 1 >= s->len) {
		*isError = false;
		*i = initial + 2;
		return NULL;
	}

	*i += 2; // skip the closing '**'
	t->text = sliceMake(s->start + initial, *i - initial);

	return t;
}

Token *makeItalicToken(Arena *arena, Slice *s, size_t *i, bool *isError) {
	Token *t = tokenMake(arena, TKN_ITALIC);
	if (!t) {
		emitError("parser", "couldn't create token");
		*isError = true;
		return NULL;
	}

	size_t initial = *i;

	(*i)++; // skip the opening '*'
	while (*i < s->len && s->start[*i] != '*') {
		(*i)++;
	}

	if (*i >= s->len) {
		*isError = false;
		*i = initial + 1;
		return NULL;
	}

	(*i)++; // skip the closing '*'
	t->text = sliceMake(s->start + initial, *i - initial);

	return t;
}

Token *makeCodespanToken(Arena *arena, Slice *s, size_t *i, bool *isError) {
	Token *t = tokenMake(arena, TKN_CODESPAN);
	if (!t) {
		emitError("parser", "couldn't create token");
		*isError = true;
		return NULL;
	}

	size_t initial = *i;

	(*i)++; // skip the opening '`'
	while (*i < s->len && s->start[*i] != '`') {
		(*i)++;
	}

	if (*i >= s->len) {
		*isError = false;
		*i = initial + 1;
		return NULL;
	}

	(*i)++; // skip the closing '`'
	t->text = sliceMake(s->start + initial, *i - initial);

	return t;
}

Token *makeHyperlinkToken(Arena *arena, Slice *s, size_t *i, bool *isError) {
	Token *t = tokenMake(arena, TKN_HYPERLINK);
	if (!t) {
		emitError("parser", "couldn't create token");
		*isError = true;
		return NULL;
	}

	size_t initial = *i;

	(*i)++; // skip the '['
	while (*i < s->len && s->start[*i] != ']') {
		(*i)++;
	}

	if (*i >= s->len) {
		*isError = false;
		*i = initial + 1;
		return NULL;
	}

	t->text = sliceMake(s->start + initial, *i - initial);

	(*i)++; // skip the ']'
	if (s->start[*i] != '(') {
		*isError = false;
		*i = initial + 1;
		return NULL;
	}

	(*i)++; // skip the '('
	size_t urlStart = *i;
	while (*i < s->len && s->start[*i] != ')') {
		(*i)++;
	}

	if (*i >= s->len) {
		*isError = false;
		*i = initial + 1;
		return NULL;
	}

	t->linkUrl = sliceMake(s->start + urlStart, *i - urlStart);
	(*i)++; // skip the ')'

	return t;
}

bool makeTokens(Arena *arena, Block *block) {
	switch (block->type) {
		case BLK_CODE:
		case BLK_HORIZONTAL_LINE: break;

		case BLK_HEADING:
		case BLK_PARAGRAPH: {
			block->tlist = arenaAlloc(arena, sizeof(TokenList));
			if (!block->tlist) {
				emitError("parser", "couldn't create token list");
				return false;
			}

			if (!tokenizeSlice(arena, block->tlist, &block->text)) {
				emitError("parser", "couldn't tokenize block slice");
				return false;
			}
			break;
		}
		case BLK_BLOCKQUOTE:
		case BLK_UNORDERED_LIST:
		case BLK_ORDERED_LIST: {
			for (int l = 0; l < block->collection.count; l++) {
				block->collection.tlists[l] = arenaAlloc(arena, sizeof(TokenList));
				if (!block->collection.tlists[l]) {
					emitError("parser", "couldn't create token list");
					return false;
				}

				if (!tokenizeSlice(arena, block->collection.tlists[l],
				                   block->collection.items + l)) {
					emitError("parser", "couldn't tokenize block slice");
					return false;
				}
			}
			break;
		}
	}

	return true;
}
