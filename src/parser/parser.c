#include <ctype.h>
#include <stdbool.h>

#include "common.h"
#include "parser.h"
#include "parser/block/block.h"

void skipWhitespace(String *f) {
	while (strchr(" \r\n\t", f->buf[f->idx]) != NULL) {
		f->idx++;
	}
}

BlockList *makeBlocks(Arena *arena, String *f) {
	BlockList *list = arenaAlloc(arena, sizeof(BlockList));
	if (!list) {
		emitError("parser", "couldn't create block list");
		return NULL;
	}

	jumpOverMetadata(f);
	skipWhitespace(f);

	while (f->buf[f->idx] != '\0') {
		Block *block = NULL;
		if (f->buf[f->idx] == '#') {
			block = makeHeadingBlock(arena, f);
		} else if (strncmp(f->buf + f->idx, "```", 3) == 0) {
			block = makeCodeBlock(arena, f);
		} else if (f->buf[f->idx] == '>') {
			block = makeBlockquoteBlock(arena, f);
		} else if (f->buf[f->idx] == '-') {
			block = makeUnorderedListBlock(arena, f);
		} else if (isdigit(f->buf[f->idx])) {
			block = makeOrderedListBlock(arena, f);
		} else {
			block = makeParagraphBlock(arena, f);
		}

		if (!block) {
			return NULL;
		}

		blockAdd(list, block);
		while (f->buf[f->idx] == '\n') {
			f->idx++;
		}
	}

	return list;
}

Block *makeHeadingBlock(Arena *arena, String *f) {
	Block *b = blockMake(arena, BLK_HEADING);
	if (!b) {
		return NULL;
	}

	int h = 0;
	while (f->buf[f->idx] == '#') {
		h++;
		f->idx++;
	}
	b->heading.level = h;

	// ensure space between heading hashes & text
	if (f->buf[f->idx] != ' ') {
		emitError("parser", "expected space after heading hashes \"%s\"", f->buf + f->idx - 1);
		return NULL;
	}
	f->idx++;

	char *textStart = f->buf + f->idx;
	int textLen = 0;
	while (f->buf[f->idx] != '\n') {
		textLen++;
		f->idx++;
	}
	f->idx++;

	b->text = sliceMake(textStart, textLen);
	return b;
}

Block *makeCodeBlock(Arena *arena, String *f) {
	Block *b = blockMake(arena, BLK_CODE);
	if (!b) {
		return NULL;
	}

	f->idx += 3;

	char *langStart = f->buf + f->idx;
	int langLen = 0;
	if (f->buf[f->idx] == '\n') {
		langStart = NULL;
	} else {
		while (f->buf[f->idx] != '\n') {
			langLen++;
			f->idx++;
		}
	}
	b->code.lang = sliceMake(langStart, langLen);

	f->idx++;

	char *codeEnd = strstr(f->buf + f->idx, "```");
	if (!codeEnd) {
		emitError("parser", "couldn't find end of code block");
		return NULL;
	}

	char *codeStart = f->buf + f->idx;
	int codeLen = 0;
	while (f->buf + f->idx != codeEnd) {
		codeLen++;
		f->idx++;
	}

	f->idx += 3;

	b->text = sliceMake(codeStart, codeLen);
	return b;
}

Block *makeParagraphBlock(Arena *arena, String *f) {
	Block *b = blockMake(arena, BLK_PARAGRAPH);
	if (!b) {
		return NULL;
	}

	char *textStart = f->buf + f->idx;
	int textLen = 0;
	while (f->buf[f->idx] != '\n') {
		textLen++;
		f->idx++;
	}
	f->idx++;

	b->text = sliceMake(textStart, textLen);
	return b;
}

Block *makeBlockquoteBlock(Arena *arena, String *f) {
	Block *b = blockMake(arena, BLK_BLOCKQUOTE);
	if (!b) {
		return NULL;
	}

	b->collection.items = arenaAlloc(arena, 32 * sizeof(Slice));
	if (!b->collection.items) {
		return NULL;
	}

	while (f->buf[f->idx] == '>') {
		f->idx++; // go past the >
		skipWhitespace(f);

		char *bquoteStart = f->buf + f->idx;

		size_t len = 0;
		while (f->buf[f->idx] != '\0' && f->buf[f->idx] != '\n') {
			len++;
			f->idx++;
		}
		if (f->buf[f->idx] == '\0') {
			emitError("parser", "couldn't find end of line in blockquote text");
			return NULL;
		}

		if (b->collection.count >= 32) {
			break;
		}
		b->collection.items[b->collection.count++] = sliceMake(bquoteStart, len);
		f->idx++;
	}

	return b;
}

Block *makeUnorderedListBlock(Arena *arena, String *f) {
	Block *b = blockMake(arena, BLK_UNORDERED_LIST);
	if (!b) {
		return NULL;
	}

	b->collection.items = arenaAlloc(arena, 32 * sizeof(Slice));
	if (!b->collection.items) {
		return NULL;
	}

	while (f->buf[f->idx] == '-') {
		if (f->buf[f->idx + 1] != ' ') {
			emitError("parser", "expected a space after the '-'");
			return NULL;
		}
		f->idx += 2; // go past the - and space
		skipWhitespace(f);

		char *ulistStart = f->buf + f->idx;

		size_t len = 0;
		while (f->buf[f->idx] != '\0' && f->buf[f->idx] != '\n') {
			len++;
			f->idx++;
		}
		if (f->buf[f->idx] == '\0') {
			emitError("parser", "couldn't find end of line in list item");
			return NULL;
		}

		if (b->collection.count >= 32) {
			break;
		}
		b->collection.items[b->collection.count++] = sliceMake(ulistStart, len);
		f->idx++;
	}

	return b;
}

Block *makeOrderedListBlock(Arena *arena, String *f) {
	Block *b = blockMake(arena, BLK_ORDERED_LIST);
	if (!b) {
		return NULL;
	}

	b->collection.items = arenaAlloc(arena, 32 * sizeof(Slice));
	if (!b->collection.items) {
		return NULL;
	}

	while (isdigit(f->buf[f->idx])) {
		// go past the number
		while (f->buf[f->idx] != '.') {
			f->idx++;
		}

		if (f->buf[f->idx + 1] != ' ') {
			emitError("parser", "expected a space after the '-'");
			return NULL;
		}
		f->idx += 2; // go past the . and space
		skipWhitespace(f);

		char *olistStart = f->buf + f->idx;

		size_t len = 0;
		while (f->buf[f->idx] != '\0' && f->buf[f->idx] != '\n') {
			len++;
			f->idx++;
		}
		if (f->buf[f->idx] == '\0') {
			emitError("parser", "couldn't find end of line in list item");
			return NULL;
		}

		if (b->collection.count >= 32) {
			break;
		}
		b->collection.items[b->collection.count++] = sliceMake(olistStart, len);
		f->idx++;
	}

	return b;
}

bool parseMarkdown(Arena *arena, String *f, Metadata **metadata, BlockList **blist) {
	*metadata = parseMetadata(arena, f);
	if (!(*metadata)) {
		emitError("parser(metadata)", "couldn't parse metadata");
		return false;
	}

	*blist = makeBlocks(arena, f);
	if (!(*blist)) {
		emitError("parser", "couldn't make blocks");
		return false;
	}

	return true;
}
