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

bool parseMetadataLine(Metadata *m, String *f) {
	int keyLen = 0;
	char *keyStart = f->buf + f->idx;
	while (f->buf[f->idx] != ':' && f->buf[f->idx] != '\n') {
		keyLen++;
		f->idx++;
	}
	if (f->buf[f->idx] == '\n') {
		emitWarning("parser(metadata)", "couldn't find entry delimiter, skipping");
		return false;
	}

	while (!isalnum(f->buf[f->idx])) {
		f->idx++;
	}

	int valueLen = 0;
	char *valueStart = f->buf + f->idx;
	while (f->buf[f->idx] != '\n' && f->buf[f->idx] != '\0') {
		valueLen++;
		f->idx++;
	}
	if (f->buf[f->idx] != '\0') {
		f->idx++;
	}

	m->keys[m->len] = sliceMake(keyStart, keyLen);
	m->values[m->len] = sliceMake(valueStart, valueLen);
	m->len++;
	return true;
}

Metadata *parseMetadata(Arena *arena, String *f) {
	Metadata *m = arenaAlloc(arena, sizeof(Metadata));
	if (!m) {
		emitError("parser(metadata)", "couldn't parse f metadata");
		return NULL;
	}

	m->len = 0;
	memset(m->keys, 0, METADATA_CAP * sizeof(Slice));
	memset(m->values, 0, METADATA_CAP * sizeof(Slice));

	// check if metadata section is present
	if (strncmp(f->buf, "---", 3) != 0) {
		emitError("parser(metadata)", "couldn't find metadata section in f");
		return NULL;
	}

	// skip to first metadata entry
	while (!isalpha(f->buf[f->idx])) {
		f->idx++;
	}

	char *metadataEnd = strstr(f->buf + f->idx, "---");
	if (!metadataEnd) {
		emitError("parser(metadata)", "couldn't find end of metadata section");
		return NULL;
	}

	while (f->buf + f->idx != metadataEnd && f->buf[f->idx] != '\0') {
		if (m->len >= METADATA_CAP) {
			break;
		}

		parseMetadataLine(m, f);

		// don't skip over the closing '---'
		while (!isalnum(f->buf[f->idx]) && f->buf + f->idx != metadataEnd) {
			f->idx++;
		}
	}

	return m;
}

bool jumpOverMetadata(String *f) {
	if (strncmp(f->buf, "---", 3) != 0) {
		emitError("parser", "metadata not found");
		return false;
	}

	f->idx = 3;

	while (f->buf[f->idx] != '\0' && strncmp(f->buf + f->idx, "---", 3) != 0) {
		f->idx++;
	}

	if (f->buf[f->idx] == '\0') {
		emitError("parser", "couldn't find end of metadata section");
		return false;
	}

	f->idx += 3;
	return true;
}
