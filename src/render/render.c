#include "render.h"
#include "parser/token.h"

int getRenderedBlockLen(Token *token) {
	int len = -1;
	switch (token->type) {
		case TKN_HEADING: {
			len = token->heading.text.len + strlen("<h1></h1>\n");
			break;
		}
		case TKN_PARAGRAPH: {
			len = token->paragraph.len + strlen("<p></p>\n");
			break;
		}
		case TKN_CODE: {
			len = token->code.text.len + token->code.lang.len + strlen("<pre><code class=\"language-\">\n</code></pre>\n");
			break;
		}
	}

	return len;
}

void renderBlock(Token *token, char *output) {
	int l = getRenderedBlockLen(token) + 1;
	char out[l];
	switch (token->type) {
		case TKN_HEADING: {
			snprintf(out, l, "<h%i>%.*s</h%i>\n", token->heading.level, (int)token->heading.text.len,
			         token->heading.text.start, token->heading.level);
			break;
		}
		case TKN_PARAGRAPH: {
			snprintf(out, l, "<p>%.*s</p>\n", (int)token->paragraph.len, token->paragraph.start);
			break;
		}
		case TKN_CODE: {
			snprintf(out, l, "<pre><code class=\"language-%.*s\">\n%.*s</code></pre>\n", (int)token->code.lang.len,
			         token->code.lang.start, (int)token->code.text.len, token->code.text.start);
			break;
		}
		default: {
			emitWarning("render", "unidentified token type encountered, skipping");
			return;
		}
	}

	strncat(output, out, l);
}

char *mdToHTML(Arena *arena, TokenList *tlist) {
	int totalLen = 0;
	for (Token *t = tlist->head; t != NULL; t = t->next) {
		int l = getRenderedBlockLen(t);
		if (l == -1) {
			emitError("render", "couldn't determine block length");
			return NULL;
		}
		totalLen += l;
	}

	char *output = arenaAlloc(arena, (totalLen + 1) * sizeof(char));
	if (!output) {
		emitError("render", "couldn't allocate output string");
		return NULL;
	}
	output[totalLen] = 0;

	for (Token *t = tlist->head; t != NULL; t = t->next) {
		renderBlock(t, output);
	}

	return output;
}
