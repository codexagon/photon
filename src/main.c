#include "common.h"
#include "memory/arena.h"
#include "sitegen/sitegen.h"

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
