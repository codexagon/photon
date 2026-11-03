#include "parser.h"
#include "common.h"

bool parseMarkdown(Arena *arena, String *f, Metadata **metadata, BlockList **blist) {
	*metadata = parseMetadata(arena, f);
	if (!(*metadata)) {
		emitError("parser(metadata)", "couldn't parse metadata");
		return false;
	}

	*blist = makeBlocks(arena, f);
	if (!(*blist)) {
		emitError("parser", "couldn't make blocks");
		return false;
	}

	for (Block *b = (*blist)->head; b != NULL; b = b->next) {
		if (!makeTokens(arena, b)) {
			emitError("parser", "couldn't tokenize blocks");
			return false;
		}
	}

	return true;
}
