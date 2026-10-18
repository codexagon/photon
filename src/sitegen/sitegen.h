#ifndef SITEGEN_H
#define SITEGEN_H

#include "memory/arena.h"
#include "parser/parser.h"
#include "util/slice.h"

struct page;
struct section;

typedef struct page {
	Slice name;
	Metadata *metadata;
	TokenList *content;
	struct section *parent;
} Page;

#define SECTION_CHILDREN_CAP 4
#define SECTION_PAGE_CAP 4

typedef struct section {
	Slice path;
	struct section *parent;
	struct section *children[SECTION_CHILDREN_CAP];
	Page *pages[SECTION_PAGE_CAP];
	int childrenLen;
	int pagesLen;
} Section;

Page *pageCreate(Arena *arena, Section *parent);
#define pageFree(page) free(page)

Section *sectionCreate(Arena *arena, Section *parent);
void sectionFree(Section *section);
bool sectionAddPage(Section *section, Page *new);
bool sectionAddSection(Section *section, Section *new);

Section *generateSiteIR(Arena *arena, const char *path);

bool getDestFilePath(char *dest, char *src);
void generateFinalSite(Arena *arena, Section *root);

#endif
