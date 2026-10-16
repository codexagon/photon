#include "util.h"

char *readFile(Arena *arena, char *filepath) {
	FILE *file = fopen(filepath, "r");
	if (!file) {
		emitError("io", "couldn't open file");
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	size_t fsize = ftell(file);
	rewind(file);

	char *buf = arenaAlloc(arena, (fsize + 1) * sizeof(char));
	if (!buf) {
		emitError("io", "couldn't allocate contents buffer");
		fclose(file);
		return NULL;
	}
	buf[fsize] = 0;

	fread(buf, fsize, 1, file);
	fclose(file);
	return buf;
}
