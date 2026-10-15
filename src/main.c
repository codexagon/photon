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

	FILE *mdfile = fopen("test.md", "r");
	if (!mdfile) {
		emitError("io", "couldn't open markdown file");
		return 2;
	}

	fseek(mdfile, 0, SEEK_END);
	size_t fsize = ftell(mdfile);
	rewind(mdfile);

	char file[fsize + 1];
	file[fsize] = 0;

	fread(file, fsize, 1, mdfile);

	Metadata *m = parseMetadata(arenaPageParse, file);
	if (!m) {
		return 1;
	}

	metadataPrint(m);
	arenaRelease(arenaPageParse);

	return 0;
}
