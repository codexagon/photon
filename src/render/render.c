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
	}

	return len;
}

void renderBlock(Block *block, char *output) {
	int l = getRenderedBlockLen(block) + 1;
	char out[l];
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
