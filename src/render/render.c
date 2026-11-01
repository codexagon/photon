#include <stdarg.h>

#include "parser/block/block.h"
#include "render.h"

int getRenderedBlockLen(Block *block) {
	int len = -1;
	switch (block->type) {
		case BLK_HEADING: {
			len = strlen("<h1></h1>\n");
			len += block->text.len;
			break;
		}
		case BLK_PARAGRAPH: {
			len = strlen("<p></p>\n");
			len += block->text.len;
			break;
		}
		case BLK_CODE: {
			len = strlen("<pre><code class=\"language-\">\n</code></pre>\n");
			len += block->text.len;
			break;
		}
		case BLK_BLOCKQUOTE: {
			len = strlen("<blockquote>\n\n</blockquote>\n");
			for (int i = 0; i < block->collection.count; i++) {
				len += block->collection.items[i].len;
			}
			break;
		}
		case BLK_UNORDERED_LIST: {
			len = strlen("<ul>\n</ul>\n") + block->collection.count * strlen("<li></li>\n");
			for (int i = 0; i < block->collection.count; i++) {
				len += block->collection.items[i].len;
			}
			break;
		}
		case BLK_ORDERED_LIST: {
			len = strlen("<ol>\n</ol>\n") + block->collection.count * strlen("<li></li>\n");
			for (int i = 0; i < block->collection.count; i++) {
				len += block->collection.items[i].len;
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

int renderBlock(Arena *arena, Block *block, String *output) {
	int written = 0;

	switch (block->type) {
		case BLK_HEADING: {
			written += stringAppend(arena, output, "<h%i>", block->heading.level);
			written += stringAppend(arena, output, "%.*s", (int)block->text.len, block->text.start);
			written += stringAppend(arena, output, "</h%i>\n", block->heading.level);
			break;
		}
		case BLK_PARAGRAPH: {
			written += stringAppend(arena, output, "<p>");
			written += stringAppend(arena, output, "%.*s", (int)block->text.len, block->text.start);
			written += stringAppend(arena, output, "</p>\n");
			break;
		}
		case BLK_CODE: {
			written += stringAppend(arena, output, "<pre><code class=\"language-%.*s\">\n", (int)block->code.lang.len,
			                        block->code.lang.start);
			written += stringAppend(arena, output, "%.*s", (int)block->text.len, block->text.start);
			written += stringAppend(arena, output, "</code></pre>\n");
			break;
		}
		case BLK_BLOCKQUOTE: {
			written += stringAppend(arena, output, "<blockquote>\n");
			for (int i = 0; i < block->collection.count; i++) {
				written +=
				    stringAppend(arena, output, "%.*s ", (int)block->collection.items[i].len, block->collection.items[i].start);
			}
			written += stringAppend(arena, output, "</blockquote>\n");
			break;
		}
		case BLK_UNORDERED_LIST: {
			written += stringAppend(arena, output, "<ul>\n");
			for (int i = 0; i < block->collection.count; i++) {
				written += stringAppend(arena, output, "<li>");
				written +=
				    stringAppend(arena, output, "%.*s", (int)block->collection.items[i].len, block->collection.items[i].start);
				written += stringAppend(arena, output, "</li>\n");
			}
			written += stringAppend(arena, output, "</ul>\n");
			break;
		}
		case BLK_ORDERED_LIST: {
			written += stringAppend(arena, output, "<ol>\n");
			for (int i = 0; i < block->collection.count; i++) {
				written += stringAppend(arena, output, "<li>");
				written +=
				    stringAppend(arena, output, "%.*s", (int)block->collection.items[i].len, block->collection.items[i].start);
				written += stringAppend(arena, output, "</li>\n");
			}
			written += stringAppend(arena, output, "</ol>\n");
			break;
		}
		default: {
			emitError("render", "unidentified block type encountered, aborting");
			return -1;
		}
	}

	return written;
}

String *mdToHTML(Arena *arena, BlockList *blist) {
	int totalLen = 0;
	for (Block *b = blist->head; b != NULL; b = b->next) {
		int l = getRenderedBlockLen(b);
		if (l == -1) {
			emitError("render", "couldn't determine block length");
			return NULL;
		}
		totalLen += l;
	}

	String *output = stringMake(arena, NULL, totalLen);
	if (!output) {
		emitError("render", "couldn't allocate output string");
		return NULL;
	}

	for (Block *b = blist->head; b != NULL; b = b->next) {
		if (!renderBlock(arena, b, output)) {
			emitError("render", "couldn't render block");
			return NULL;
		}
	}

	return output;
}
