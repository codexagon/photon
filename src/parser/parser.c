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

TokenList *tokenizeBlocks(Arena *arena, char *file) {
	TokenList *list = arenaAlloc(arena, sizeof(TokenList));
	if (!list) {
		emitError("parser", "couldn't create token list");
		return NULL;
	}

	char *f = jumpOverMetadata(file);

	// skip leading whitespace
	while (strchr(" \r\n\t", *f) != NULL) {
		f++;
	}

	while (*f != '\0') {
		Token *token = NULL;
		if (*f == '#') {
			token = tokenizeHeading(arena, &f);
		} else if (strncmp(f, "```", 3) == 0) {
			token = tokenizeCodeBlock(arena, &f);
		} else {
			token = tokenizeParagraph(arena, &f);
		}

		if (!token) {
			emitError("parser", "couldn't tokenize");
			return NULL;
		}

		tokenAdd(list, token);
		while (*f == '\n') {
			f++;
		}
	}

	return list;
}

Token *tokenizeHeading(Arena *arena, char **f) {
	Token *t = tokenMake(arena, TKN_HEADING);
	if (!t) {
		return NULL;
	}

	int h = 0;
	while (**f == '#') {
		h++;
		(*f)++;
	}
	t->heading.level = h;

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

	t->heading.text = sliceMake(textStart, textLen);
	return t;
}

Token *tokenizeCodeBlock(Arena *arena, char **f) {
	Token *t = tokenMake(arena, TKN_CODE);
	if (!t) {
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
	t->code.lang = sliceMake(langStart, langLen);

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

	t->code.text = sliceMake(codeStart, codeLen);
	return t;
}

Token *tokenizeParagraph(Arena *arena, char **f) {
	Token *t = tokenMake(arena, TKN_PARAGRAPH);
	if (!t) {
		return NULL;
	}

	char *textStart = *f;
	int textLen = 0;
	while (**f != '\n') {
		textLen++;
		(*f)++;
	}
	(*f)++;

	t->paragraph = sliceMake(textStart, textLen);
	return t;
}
