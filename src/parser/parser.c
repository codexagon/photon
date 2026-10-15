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

Metadata *parseMetadata(Arena *arena, const char *file) {
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
