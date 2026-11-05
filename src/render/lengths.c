#include "render.h"

int getRenderedTokenLen(Token *t) {
	int len = 0;
	switch (t->type) {
		case TKN_BOLD: len = strlen("<strong></strong>"); break;
		case TKN_ITALIC: len = strlen("<em></em>"); break;
		case TKN_CODESPAN: {
			len = strlen("<code></code>");
			for (size_t i = 0; i < t->text.len; i++) {
				int l = 1;
				getEscapedChar(t->text.start[i], &l);
				len += l;
			}
			break;
		}
	}

	return len;
}

int getRenderedTokenListLen(TokenList *tlist) {
	int len = 0;
	for (Token *t = tlist->head; t != NULL; t = t->next) {
		len += getRenderedTokenLen(t);
	}

	return len;
}

int getRenderedBlockLen(Block *block) {
	int len = -1;
	switch (block->type) {
		case BLK_HEADING: {
			len = strlen("<h1></h1>\n");
			len += block->text.len;
			len += getRenderedTokenListLen(block->tlist);
			break;
		}
		case BLK_PARAGRAPH: {
			len = strlen("<p></p>\n");
			len += block->text.len;
			len += getRenderedTokenListLen(block->tlist);
			break;
		}
		case BLK_CODE: {
			len = strlen("<pre><code class=\"language-\">\n</code></pre>\n") * 2;
			len += block->text.len;
			break;
		}
		case BLK_BLOCKQUOTE: {
			len = strlen("<blockquote>\n\n</blockquote>\n");
			for (int i = 0; i < block->collection.count; i++) {
				len += block->collection.items[i].len;
				len += getRenderedTokenListLen(block->collection.tlists[i]);
			}
			break;
		}
		case BLK_UNORDERED_LIST: {
			len = strlen("<ul>\n</ul>\n") + block->collection.count * strlen("<li></li>\n");
			for (int i = 0; i < block->collection.count; i++) {
				len += block->collection.items[i].len;
				len += getRenderedTokenListLen(block->collection.tlists[i]);
			}
			break;
		}
		case BLK_ORDERED_LIST: {
			len = strlen("<ol>\n</ol>\n") + block->collection.count * strlen("<li></li>\n");
			for (int i = 0; i < block->collection.count; i++) {
				len += block->collection.items[i].len;
				len += getRenderedTokenListLen(block->collection.tlists[i]);
			}
			break;
		}
	}

	if (len == -1) {
		emitError("render", "unidentified block encountered");
		return -1;
	}

	return len;
}

char *getEscapedChar(char c, int *l) {
	switch (c) {
		case '"': {
			*l = strlen("&quot;");
			return "&quot;";
		}
		case '&': {
			*l = strlen("&amp;");
			return "&amp;";
		}
		case '\'': {
			*l = strlen("&#39;");
			return "&#39;";
		}
		case '<': {
			*l = strlen("&lt;");
			return "&lt;";
		}
		case '>': {
			*l = strlen("&gt;");
			return "&gt;";
		}
		default: {
			return NULL;
		}
	}
}
