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
	switch (token->type) {
		case TKN_HEADING: {
			switch (token->heading.level) {
				case 1: strcat(output, "<h1>"); break;
				case 2: strcat(output, "<h2>"); break;
				case 3: strcat(output, "<h3>"); break;
				case 4: strcat(output, "<h4>"); break;
				case 5: strcat(output, "<h5>"); break;
				case 6: strcat(output, "<h6>"); break;
			}
			strncat(output, token->heading.text.start, token->heading.text.len);
			switch (token->heading.level) {
				case 1: strcat(output, "</h1>\n"); break;
				case 2: strcat(output, "</h2>\n"); break;
				case 3: strcat(output, "</h3>\n"); break;
				case 4: strcat(output, "</h4>\n"); break;
				case 5: strcat(output, "</h5>\n"); break;
				case 6: strcat(output, "</h6>\n"); break;
			}
			break;
		}
		case TKN_PARAGRAPH: {
			strcat(output, "<p>");
			strncat(output, token->paragraph.start, token->paragraph.len);
			strcat(output, "</p>\n");
			break;
		}
		case TKN_CODE: {
			strcat(output, "<pre><code class=\"language-");
			strncat(output, token->code.lang.start, token->code.lang.len);
			strcat(output, "\">\n");
			strncat(output, token->code.text.start, token->code.text.len);
			strcat(output, "</code></pre>\n");
			break;
		}
		default: {
			emitWarning("render", "unidentified token type encountered, skipping");
			break;
		}
	}
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
