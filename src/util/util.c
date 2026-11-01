#include <errno.h>
#include <sys/stat.h>

#include "util.h"

String *readFile(Arena *arena, char *filepath) {
	FILE *file = fopen(filepath, "r");
	if (!file) {
		emitError("io", "couldn't open file");
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	size_t fsize = ftell(file);
	rewind(file);

	String *str = stringMake(arena, NULL, fsize);
	if (!str) {
		emitError("io", "couldn't create contents string");
		fclose(file);
		return NULL;
	}

	char tmp[fsize + 1];
	tmp[fsize] = '\0';
	fread(tmp, fsize, 1, file);
	stringWrite(arena, str, "%s", tmp);

	fclose(file);
	return str;
}

// handle EEXIST error separately so that we don't have to remove dest dir everytime
bool createDir(const char *path) {
	if (mkdir(path, 0755) == 0) {
		return true;
	}

	if (errno != EEXIST) {
		return false;
	}

	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}
