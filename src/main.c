#include "common.h"
#include "memory/arena.h"
#include "sitegen/sitegen.h"

int main(void) {
	Arena *arena = arenaCreate(1024 * 512); // 0.5MB
	if (!arena) {
		emitError("main", "couldn't create arena");
		return 1;
	}

	Section *root = generateSiteIR(arena, "content/");
	if (!root) {
		return 2;
	}

	displaySiteIR(root);

	arenaRelease(arena);
	return 0;
}
