#include <ctype.h>
#include <stdbool.h>

#include "common.h"
#include "parser.h"

// for testing
void metadataPrint(Metadata *m) {
	for (int i = 0; i < m->len; i++) {
		slicePrint(m->keys[i]);
		printf(": ");
		slicePrint(m->values[i]);
		printf("\n");
	}
}

bool parseMetadataLine(Metadata *m, char **f) {
	int keyLen = 0;
	char *keyStart = *f;
	while (**f != ':' && **f != '\n') {
		keyLen++;
		(*f)++;
	}
	if (**f == '\n') {
		emitWarning("parser(metadata)", "couldn't find entry delimiter, skipping");
		return false;
	}

	while (!isalnum(**f)) {
		(*f)++;
	}

	int valueLen = 0;
	char *valueStart = *f;
	while (**f != '\n' && **f != '\0') {
		valueLen++;
		(*f)++;
	}
	if (**f != '\0') {
		(*f)++;
	}

	m->keys[m->len] = sliceMake(keyStart, keyLen);
	m->values[m->len] = sliceMake(valueStart, valueLen);
	m->len++;
	return true;
}

Metadata *parseMetadata(Arena *arena, char *file) {
	Metadata *m = arenaAlloc(arena, sizeof(Metadata));
	if (!m) {
		emitError("parser(metadata)", "couldn't parse file metadata");
		return NULL;
	}

	m->len = 0;
	memset(m->keys, 0, METADATA_CAP * sizeof(Slice));
	memset(m->values, 0, METADATA_CAP * sizeof(Slice));

	// check if metadata section is present
	if (strncmp(file, "---", 3) != 0) {
		emitError("parser(metadata)", "couldn't find metadata section in file");
		return NULL;
	}

	// skip to first metadata entry
	char *f = file;
	while (!isalpha(*f)) {
		f++;
	}

	char *metadataEnd = strstr(f, "---");
	if (!metadataEnd) {
		emitError("parser(metadata)", "couldn't find end of metadata section");
		return NULL;
	}

	while (f != metadataEnd && *f != '\0') {
		if (m->len >= METADATA_CAP) {
			break;
		}

		parseMetadataLine(m, &f);

		// don't skip over the closing '---'
		while (!isalnum(*f) && f != metadataEnd) {
			f++;
		}
	}

	return m;
}

char *jumpOverMetadata(char *file) {
	if (strncmp(file, "---", 3) != 0) {
		emitWarning("parser", "metadata not found, no-op");
		return file;
	}

	char *metadataEnd = strstr(file + 3, "---");
	if (!metadataEnd) {
		emitError("parser", "couldn't find end of metadata section");
		return NULL;
	}

	return metadataEnd + 4;
}

char *skipWhitespace(char *f) {
	while (strchr(" \r\n\t", *f) != NULL) {
		f++;
	}
	return f;
}

BlockList *makeBlocks(Arena *arena, char *file) {
	BlockList *list = arenaAlloc(arena, sizeof(BlockList));
	if (!list) {
		emitError("parser", "couldn't create block list");
		return NULL;
	}

	char *f = jumpOverMetadata(file);

	// skip leading whitespace
	f = skipWhitespace(f);

	while (*f != '\0') {
		Block *block = NULL;
		if (*f == '#') {
			block = makeHeadingBlock(arena, &f);
		} else if (strncmp(f, "```", 3) == 0) {
			block = makeCodeBlock(arena, &f);
		} else if (*f == '>') {
			block = makeBlockquoteBlock(arena, &f);
		} else {
			block = makeParagraphBlock(arena, &f);
		}

		if (!block) {
			emitError("parser", "couldn't tokenize");
			return NULL;
		}

		blockAdd(list, block);
		while (*f == '\n') {
			f++;
		}
	}

	return list;
}

Block *makeHeadingBlock(Arena *arena, char **f) {
	Block *b = blockMake(arena, BLK_HEADING);
	if (!b) {
		return NULL;
	}

	int h = 0;
	while (**f == '#') {
		h++;
		(*f)++;
	}
	b->heading.level = h;

	// ensure space between heading hashes & text
	if (**f != ' ') {
		emitError("parser", "expected space after heading hashes");
		return NULL;
	}
	(*f)++;

	char *textStart = *f;
	int textLen = 0;
	while (**f != '\n') {
		textLen++;
		(*f)++;
	}
	(*f)++;

	b->heading.text = sliceMake(textStart, textLen);
	return b;
}

Block *makeCodeBlock(Arena *arena, char **f) {
	Block *b = blockMake(arena, BLK_CODE);
	if (!b) {
		return NULL;
	}

	(*f) += 3;

	char *langStart = *f;
	int langLen = 0;
	if (**f == '\n') {
		langStart = NULL;
	} else {
		while (**f != '\n') {
			langLen++;
			(*f)++;
		}
	}
	b->code.lang = sliceMake(langStart, langLen);

	(*f)++;

	char *codeEnd = strstr(*f, "```");
	if (!codeEnd) {
		emitError("parser", "couldn't find end of code block");
		return NULL;
	}

	char *codeStart = *f;
	int codeLen = 0;
	while (*f != codeEnd) {
		codeLen++;
		(*f)++;
	}

	(*f) += 3;

	b->code.text = sliceMake(codeStart, codeLen);
	return b;
}

Block *makeParagraphBlock(Arena *arena, char **f) {
	Block *b = blockMake(arena, BLK_PARAGRAPH);
	if (!b) {
		return NULL;
	}

	char *textStart = *f;
	int textLen = 0;
	while (**f != '\n') {
		textLen++;
		(*f)++;
	}
	(*f)++;

	b->paragraph = sliceMake(textStart, textLen);
	return b;
}

Block *makeBlockquoteBlock(Arena *arena, char **f) {
	Block *b = blockMake(arena, BLK_BLOCKQUOTE);
	if (!b) {
		return NULL;
	}

	b->bquote.lines = arenaAlloc(arena, 32 * sizeof(Slice));
	if (!b->bquote.lines) {
		return NULL;
	}

	while (**f == '>') {
		(*f)++; // go past the >
		*f = skipWhitespace(*f);
		char *end = strchr(*f, '\n');
		if (!end) {
			emitError("parser", "couldn't find end of line in blockquote text");
			return NULL;
		}

		size_t len = end - *f;
		if (b->bquote.count >= 32) {
			break;
		}
		b->bquote.lines[b->bquote.count++] = sliceMake(*f, len);
		*f += len + 1;
	}

	return b;
}
