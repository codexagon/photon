#include "common.h"
#include "memory/arena.h"
#include "parser/parser.h"
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

	Arena *arenaPageParse = arenaCreate(4096);
	if (!arenaPageParse) {
		return 1;
	}

	const char *file = "---\nhello: world\nnever gonna: give you up\nnever gonna: let you down\n---";

	Metadata *m = parseMetadata(arenaPageParse, file);
	if (!m) {
		return 1;
	}

	metadataPrint(m);
	arenaRelease(arenaPageParse);

	return 0;
}
