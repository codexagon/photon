#include "common.h"
#include "memory/arena.h"
#include "sitegen/sitegen.h"

int main(void) {
	Arena *arenaSiteIR = arenaCreate(8192);
	if (!arenaSiteIR) {
		return 1;
	}

	Section *root = generateSiteIR(arenaSiteIR, "content/");
	if (!root) {
		return 2;
	}

	displaySiteIR(root, 0);

	arenaRelease(arenaSiteIR);

	return 0;
}
