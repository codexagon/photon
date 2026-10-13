#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "common.h"
#include "sitegen.h"
#include "util/slice.h"

Page *pageCreate(Arena *arena, Section *parent) {
	Page *page = arenaAlloc(arena, sizeof(Page));
	page->name = sliceMake(NULL, 0);
	page->metadata = sliceMake(NULL, 0);
	page->parent = parent;
	return page;
}

Section *sectionCreate(Arena *arena, Section *parent) {
	Section *section = arenaAlloc(arena, sizeof(Section));
	if (!section) {
		emitError("failed to create section");
		return NULL;
	}
	section->path = sliceMake(NULL, 0);

	section->childrenLen = 0;
	section->children = arenaAlloc(arena, SECTION_CHILDREN_CAP * sizeof(Section *));
	if (!section->children) {
		emitError("failed to create section children array");
		return NULL;
	}

	section->pagesLen = 0;
	section->pages = arenaAlloc(arena, SECTION_PAGE_CAP * sizeof(Page *));
	if (!section->pages) {
		emitError("failed to create section pages array");
		return NULL;
	}

	section->parent = parent;
	return section;
}

void sectionFree(Section *section) {
	for (int i = 0; i < section->pagesLen; i++) {
		pageFree(section->pages[i]);
		section->pages[i] = NULL;
	}

	for (int i = 0; i < section->childrenLen; i++) {
		sectionFree(section->children[i]);
		section->children[i] = NULL;
	}

	free(section);
}

bool sectionAddPage(Section *section, Page *new) {
	if (section->pagesLen >= SECTION_PAGE_CAP) {
		emitError("exceeded section page limit");
		return false;
	}

	section->pages[section->pagesLen++] = new;
	return true;
}

bool sectionAddSection(Section *section, Section *new) {
	if (section->childrenLen >= SECTION_CHILDREN_CAP) {
		emitError("exceeded section children limit");
		return false;
	}

	section->children[section->childrenLen++] = new;
	return true;
}

bool generateRecursive(Arena *arena, Section *root, const char *path) {
	DIR *cdir = opendir(path);
	if (!cdir) {
		emitError("failed to open directory");
		return false;
	}

	struct dirent *entry;
	while ((entry = readdir(cdir)) != NULL) {
		if (strncmp(entry->d_name, ".", 1) == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		int pl = strlen(path);
		int l = strlen(entry->d_name);

		switch (entry->d_type) {
			case DT_REG: {
				char *name = arenaAlloc(arena, (l - 2) * sizeof(char));
				if (!name) {
					return false;
				}
				strncpy(name, entry->d_name, l - 3);

				Page *newPage = pageCreate(arena, root);
				if (!sectionAddPage(root, newPage)) {
					pageFree(newPage);
					emitError("couldn't add page");
					return false;
				}
				newPage->name = sliceMake(name, l - 3);
				break;
			}
			case DT_DIR: {
				char *pathName = arenaAlloc(arena, (pl + l + 2) * sizeof(char));
				if (!pathName) {
					return false;
				}

				Section *newSection = sectionCreate(arena, root);
				if (!sectionAddSection(root, newSection)) {
					sectionFree(newSection);
					emitError("couldn't add section");
					return false;
				}

				strcpy(pathName, path);
				strcat(pathName, entry->d_name);
				strcat(pathName, "/");

				newSection->path = sliceMake(pathName, pl + l + 1);

				generateRecursive(arena, newSection, pathName);
				break;
			}
		}
	}

	closedir(cdir);

	return true;
}

Section *generateSiteIR(Arena *arena, const char *path) {
	Section *root = sectionCreate(arena, NULL);
	if (!generateRecursive(arena, root, path)) {
		emitError("couldn't generate site IR");
		return NULL;
	}
	return root;
}

// for testing
void displaySiteIR(Section *root, int depth) {
	for (int i = 0; i < root->pagesLen; i++) {
		for (int j = 0; j < depth; j++) {
			printf("    ");
		}
		printf("[page ");
		slicePrint(root->pages[i]->name);
		printf("]\n");
	}

	for (int i = 0; i < root->childrenLen; i++) {
		for (int j = 0; j < depth; j++) {
			printf("    ");
		}
		printf("[");
		slicePrint(root->children[i]->path);
		printf("]\n");
		displaySiteIR(root->children[i], depth + 1);
	}
}
