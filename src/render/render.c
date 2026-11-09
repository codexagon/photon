#include <stdarg.h>

#include "parser/block/block.h"
#include "render.h"

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

int writeEscapedChars(Arena *arena, String *output, Slice *content) {
	int written = 0;

	for (size_t i = 0; i < content->len; i++) {
		int l = 1;
		char *esc = getEscapedChar(content->start[i], &l);
		if (esc) {
			stringAppend(arena, output, "%s", esc);
		} else {
			stringAppendChar(arena, output, content->start[i]);
		}
		written += l;
	}

	return written;
}

int renderToken(Arena *arena, Token *token, String *output) {
	switch (token->type) {
		case TKN_BOLD: {
			Slice content = sliceMake(token->text.start + 2, token->text.len - 4);
			stringAppend(arena, output, "<strong>");
			writeEscapedChars(arena, output, &content);
			stringAppend(arena, output, "</strong>");
			break;
		}
		case TKN_ITALIC: {
			Slice content = sliceMake(token->text.start + 1, token->text.len - 2);
			stringAppend(arena, output, "<em>");
			writeEscapedChars(arena, output, &content);
			stringAppend(arena, output, "</em>");
			break;
		}
		case TKN_CODESPAN: {
			Slice content = sliceMake(token->text.start + 1, token->text.len - 2);
			stringAppend(arena, output, "<code>");
			writeEscapedChars(arena, output, &content);
			stringAppend(arena, output, "</code>");
			break;
		}
	}

	return token->text.len;
}

int renderBlockContents(Arena *arena, Slice *text, TokenList *tlist, String *output) {
	int written = 0;
	Token *curToken = tlist->head;

	size_t i = 0;
	while (i < text->len) {
		Slice content = sliceMake(text->start + i, 0);

		size_t initial = i;
		while (curToken ? text->start + i != curToken->text.start : i < text->len) {
			i++;
		}

		content.len = i - initial;
		written += writeEscapedChars(arena, output, &content);

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
			written += stringAppend(arena, output, "<pre><code class=\"language-%.*s\">", (int)block->code.lang.len,
			                        block->code.lang.start);
			written += writeEscapedChars(arena, output, &block->text);
			written += stringAppend(arena, output, "</code></pre>\n");
			break;
		}
		case BLK_BLOCKQUOTE: {
			written += stringAppend(arena, output, "<blockquote>");
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
	String *output = stringMake(arena, NULL, 1024);
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

	output->len--;
	return output;
}
