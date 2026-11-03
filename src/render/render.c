#include <stdarg.h>

#include "parser/block/block.h"
#include "render.h"

int renderToken(Arena *arena, Token *token, String *output) {
	switch (token->type) {
		case TKN_BOLD: {
			Slice content = sliceMake(token->text.start + 2, token->text.len - 4);
			stringAppend(arena, output, "<strong>");
			stringAppendSlice(arena, output, content);
			stringAppend(arena, output, "</strong>");
			break;
		}
		case TKN_ITALIC: {
			Slice content = sliceMake(token->text.start + 1, token->text.len - 2);
			stringAppend(arena, output, "<em>");
			stringAppendSlice(arena, output, content);
			stringAppend(arena, output, "</em>");
			break;
		}
		case TKN_CODESPAN: {
			Slice content = sliceMake(token->text.start + 1, token->text.len - 2);
			stringAppend(arena, output, "<code>");
			stringAppendSlice(arena, output, content);
			stringAppend(arena, output, "</code>");
			break;
		}
	}

	return token->text.len;
}

int renderBlockContents(Arena *arena, Slice *text, TokenList *tlist, String *output) {
	int written = 0;

	Token *curToken = tlist->head;

	// improve this section, use slices instead of comparing every character
	size_t i = 0;
	while (i < text->len) {
		while (curToken ? text->start + i != curToken->text.start : i < text->len) {
			written += stringAppendChar(arena, output, text->start[i]);
			i++;
		}

		if (curToken) {
			i += renderToken(arena, curToken, output);
			curToken = curToken->next;
		}
	}

	return written;
}

int renderBlock(Arena *arena, Block *block, String *output) {
	int written = 0;

	switch (block->type) {
		case BLK_HEADING: {
			written += stringAppend(arena, output, "<h%i>", block->heading.level);
			written += renderBlockContents(arena, &block->text, block->tlist, output);
			written += stringAppend(arena, output, "</h%i>\n", block->heading.level);
			break;
		}
		case BLK_PARAGRAPH: {
			written += stringAppend(arena, output, "<p>");
			written += renderBlockContents(arena, &block->text, block->tlist, output);
			written += stringAppend(arena, output, "</p>\n");
			break;
		}
		case BLK_CODE: {
			written += stringAppend(arena, output, "<pre><code class=\"language-%.*s\">\n", (int)block->code.lang.len,
			                        block->code.lang.start);
			written += stringAppendSlice(arena, output, block->text);
			written += stringAppend(arena, output, "</code></pre>\n");
			break;
		}
		case BLK_BLOCKQUOTE: {
			written += stringAppend(arena, output, "<blockquote>\n");
			for (int i = 0; i < block->collection.count; i++) {
				written += renderBlockContents(arena, &block->collection.items[i], block->collection.tlists[i], output);
				written += stringAppendChar(arena, output, ' ');
			}
			written += stringAppend(arena, output, "</blockquote>\n");
			break;
		}
		case BLK_UNORDERED_LIST: {
			written += stringAppend(arena, output, "<ul>\n");
			for (int i = 0; i < block->collection.count; i++) {
				written += stringAppend(arena, output, "<li>");
				written += renderBlockContents(arena, &block->collection.items[i], block->collection.tlists[i], output);
				written += stringAppend(arena, output, "</li>\n");
			}
			written += stringAppend(arena, output, "</ul>\n");
			break;
		}
		case BLK_ORDERED_LIST: {
			written += stringAppend(arena, output, "<ol>\n");
			for (int i = 0; i < block->collection.count; i++) {
				written += stringAppend(arena, output, "<li>");
				written += renderBlockContents(arena, &block->collection.items[i], block->collection.tlists[i], output);
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
