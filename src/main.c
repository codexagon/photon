#include <stdarg.h>

#include "common.h"
#include "memory/arena.h"
#include "sitegen/sitegen.h"

void emitError(const char *module, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "\033[38;5;160;1m[ERROR] %s: ", module);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\033[0m\n");
	va_end(args);
}

void emitWarning(const char *module, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "\033[38;5;214;1m[WARN] %s: ", module);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\033[0m\n");
	va_end(args);
}

int main(void) {
	Arena *arena = arenaCreate(1024 * 1024 * 1024); // 1 GB
	if (!arena) {
		emitError("main", "couldn't create arena");
		return 1;
	}

	Section *root = generateSiteIR(arena, "content/");
	if (!root) {
		emitError("main", "couldn't generate site IR");
		return 2;
	}

	Arena *arenaPage = arenaCreate(1024 * 1024 * 1024);
	if (!arenaPage) {
		emitError("main", "couldn't create arena");
		return 1;
	}

	generateFinalSite(arenaPage, root);

	arenaRelease(arenaPage);
	arenaRelease(arena);
	return 0;
}
