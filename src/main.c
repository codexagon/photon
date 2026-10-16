#include "common.h"
#include "memory/arena.h"
#include "parser/parser.h"
#include "sitegen/sitegen.h"

int main(void) {
	Arena *arenaPageParse = arenaCreate(4096);
	if (!arenaPageParse) {
		return 1;
	}

	FILE *mdfile = fopen("content/test.md", "r");
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

	TokenList *tlist = tokenizeBlocks(arenaPageParse, file);
	if (!tlist) {
		arenaRelease(arenaPageParse);
		return 3;
	}

	for (Token *t = tlist->head; t != NULL; t = t->next) {
		tokenPrint(t);
	}

	arenaRelease(arenaPageParse);

	return 0;
}
