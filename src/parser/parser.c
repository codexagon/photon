#include <ctype.h>

#include "common.h"
#include "parser.h"

// for testing
void metadataPrint(Metadata *m) {
	printf("metadata\n");
	for (int i = 0; i < m->len; i++) {
		slicePrint(m->keys[i]);
		printf(": ");
		slicePrint(m->values[i]);
		printf("\n");
	}
}

Metadata *parseMetadata(Arena *arena, const char *file) {
	Metadata *m = arenaAlloc(arena, sizeof(Metadata));
	if (!m) {
		emitError("couldn't parse file metadata");
		return NULL;
	}

	m->len = 0;
	memset(m->keys, 0, METADATA_CAP * sizeof(Slice));
	memset(m->values, 0, METADATA_CAP * sizeof(Slice));

	char *f = strstr(file, "---");
	if (!f) {
		emitError("couldn't find metadata section in file");
		return NULL;
	}

	f += 4;

	while (*f != '-') {
		if (m->len >= METADATA_CAP) {
			break;
		}

		int keyLen = 0;
		char *keyStart = f;
		while (*f != ':' && *f != '\0') {
			keyLen++;
			f++;
		}
		if (*f == '\0') {
			emitError("couldn't find ':'");
			return NULL;
		}

		while (!isalnum(*f)) {
			f++;
		}

		int valueLen = 0;
		char *valueStart = f;
		while (*f != '\n' && *f != '\0') {
			valueLen++;
			f++;
		}
		if (*f != '\0') {
			f++;
		}

		m->keys[m->len] = sliceMake(keyStart, keyLen);
		m->values[m->len] = sliceMake(valueStart, valueLen);
		m->len++;
	}

	return m;
}
