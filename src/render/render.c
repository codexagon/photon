#include "render.h"

int getRenderedBlockLen(Block *block) {
	int len = -1;
	switch (block->type) {
		case BLK_HEADING: {
			len = block->heading.text.len + strlen("<h1></h1>\n");
			break;
		}
		case BLK_PARAGRAPH: {
			len = block->paragraph.len + strlen("<p></p>\n");
			break;
		}
		case BLK_CODE: {
			len = block->code.text.len + block->code.lang.len + strlen("<pre><code class=\"language-\">\n</code></pre>\n");
			break;
		}
		case BLK_BLOCKQUOTE: {
			len = strlen("<blockquote>\n\n</blockquote>\n");
			for (int i = 0; i < block->bquote.count; i++) {
				len += block->bquote.lines[i].len + 1;
			}
			break;
		}
		case BLK_UNORDERED_LIST: {
			len = strlen("<ul>\n</ul>\n") + block->list.count * strlen("<li></li>\n");
			for (int i = 0; i < block->list.count; i++) {
				len += block->list.items[i].len;
			}
			break;
		}
		case BLK_ORDERED_LIST: {
			len = strlen("<ol>\n</ol>\n") + block->list.count * strlen("<li></li>\n");
			for (int i = 0; i < block->list.count; i++) {
				len += block->list.items[i].len;
			}
			break;
		}
	}

	return len;
}

void renderBlock(Block *block, char *output) {
	int l = getRenderedBlockLen(block) + 1;
	char out[l];
	out[0] = '\0';
	switch (block->type) {
		case BLK_HEADING: {
			snprintf(out, l, "<h%i>%.*s</h%i>\n", block->heading.level, (int)block->heading.text.len,
			         block->heading.text.start, block->heading.level);
			break;
		}
		case BLK_PARAGRAPH: {
			snprintf(out, l, "<p>%.*s</p>\n", (int)block->paragraph.len, block->paragraph.start);
			break;
		}
		case BLK_CODE: {
			snprintf(out, l, "<pre><code class=\"language-%.*s\">\n%.*s</code></pre>\n", (int)block->code.lang.len,
			         block->code.lang.start, (int)block->code.text.len, block->code.text.start);
			break;
		}
		case BLK_BLOCKQUOTE: {
			strcat(out, "<blockquote>\n");
			for (int i = 0; i < block->bquote.count; i++) {
				strncat(out, block->bquote.lines[i].start, block->bquote.lines[i].len);
				strcat(out, " ");
			}
			strcat(out, "\n</blockquote>\n");
			break;
		}
		case BLK_UNORDERED_LIST: {
			strcat(out, "<ul>\n");
			for (int i = 0; i < block->list.count; i++) {
				strcat(out, "<li>");
				strncat(out, block->list.items[i].start, block->list.items[i].len);
				strcat(out, "</li>\n");
			}
			strcat(out, "</ul>\n");
			break;
		}
		case BLK_ORDERED_LIST: {
			strcat(out, "<ol>\n");
			for (int i = 0; i < block->list.count; i++) {
				strcat(out, "<li>");
				strncat(out, block->list.items[i].start, block->list.items[i].len);
				strcat(out, "</li>\n");
			}
			strcat(out, "</ol>\n");
			break;
		}
		default: {
			emitWarning("render", "unidentified block type encountered, skipping");
			return;
		}
	}

	strncat(output, out, l);
}

char *mdToHTML(Arena *arena, BlockList *blist) {
	int totalLen = 0;
	for (Block *b = blist->head; b != NULL; b = b->next) {
		int l = getRenderedBlockLen(b);
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

	for (Block *b = blist->head; b != NULL; b = b->next) {
		renderBlock(b, output);
	}

	return output;
}
