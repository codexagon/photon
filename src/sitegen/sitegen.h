#ifndef SITEGEN_H
#define SITEGEN_H

#include "memory/arena.h"
#include "parser/parser.h"
#include "util/str.h"

struct page;
struct section;

typedef struct nodepage {
	struct page *page;
	struct nodepage *next;
} PageNode;

typedef struct nodesection {
	struct section *section;
	struct nodesection *next;
} SectionNode;

typedef struct page {
	Slice name;
	Metadata *metadata;
	BlockList *content;
	struct section *parent;
} Page;

typedef struct section {
	Slice path;
	struct section *parent;
	SectionNode *children;
	PageNode *pages;
} Section;

Page *pageCreate(Arena *arena, Section *parent);

Section *sectionCreate(Arena *arena, Section *parent);
bool sectionAddPage(Arena *arena, Section *section, Page *new);
bool sectionAddSection(Arena *arena, Section *section, Section *new);

Section *generateSiteIR(Arena *arena, const char *path, char *destDirName);

bool parseSite(Arena *arena, Section *root);

bool getDestFilePath(char *dest, char *src, char *destDirName);
void generateFinalSite(Arena *arena, Section *root, char *destDirName);

#endif
