#include <dirent.h>
#include <sys/stat.h>

#include "common.h"
#include "render/template.h"
#include "sitegen.h"
#include "util/slice.h"
#include "util/util.h"

Page *pageCreate(Arena *arena, Section *parent) {
	Page *page = arenaAlloc(arena, sizeof(Page));
	if (!page) {
		emitError("sitegen", "failed to create page");
		return NULL;
	}
	page->name = sliceMake(NULL, 0);
	page->metadata = NULL;
	page->content = NULL;
	page->parent = parent;
	return page;
}

Section *sectionCreate(Arena *arena, Section *parent) {
	Section *section = arenaAlloc(arena, sizeof(Section));
	if (!section) {
		emitError("sitegen", "failed to create section");
		return NULL;
	}
	section->path = sliceMake(NULL, 0);

	section->childrenLen = 0;
	memset(section->children, 0, SECTION_CHILDREN_CAP * sizeof(Section *));

	section->pagesLen = 0;
	memset(section->pages, 0, SECTION_PAGE_CAP * sizeof(Page *));

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
		emitError("sitegen", "exceeded section page limit");
		return false;
	}

	section->pages[section->pagesLen++] = new;
	return true;
}

bool sectionAddSection(Section *section, Section *new) {
	if (section->childrenLen >= SECTION_CHILDREN_CAP) {
		emitError("sitegen", "exceeded section children limit");
		return false;
	}

	section->children[section->childrenLen++] = new;
	return true;
}

// builds site IR and generates destination dir structure too
bool generateRecursive(Arena *arena, Section *root, const char *path, const char *destPath) {
	DIR *cdir = opendir(path);
	if (!cdir) {
		emitError("sitegen", "failed to open directory");
		return false;
	}

	struct dirent *entry;
	while ((entry = readdir(cdir)) != NULL) {
		if (strncmp(entry->d_name, ".", 1) == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		int pl = strlen(path);
		int l = strlen(entry->d_name);

		// store full path + current file/dir name, use for both cases
		char *fullName = arenaAlloc(arena, (pl + l + 2) * sizeof(char));
		if (!fullName) {
			emitError("sitegen", "couldn't allocate full path string");
			return false;
		}

		strcpy(fullName, path);
		strcat(fullName, entry->d_name);

		switch (entry->d_type) {
			case DT_REG: {
				char *file = readFile(arena, fullName);
				if (!file) {
					emitError("sitegen", "couldn't read file");
					return false;
				}

				Page *newPage = pageCreate(arena, root);
				if (!sectionAddPage(root, newPage)) {
					pageFree(newPage);
					emitError("sitegen", "couldn't add page");
					return false;
				}
				char *nameStart = strrchr(fullName, '/') + 1;
				newPage->name = sliceMake(nameStart, l - 3);

				newPage->metadata = parseMetadata(arena, file);
				newPage->content = tokenizeBlocks(arena, file);

				// create the dest dir for this file if its name isn't root.md
				if (!sliceEqual(newPage->name, sliceMake("root", 4))) {
					int n = newPage->name.len + strlen(destPath) + 1;
					char destDir[n];
					for (int i = 0; i < n; i++) {
						destDir[i] = '\0';
					}
					strcat(destDir, destPath);
					strncat(destDir, newPage->name.start, newPage->name.len);
					if (mkdir(destDir, 0755) == -1) {
						emitError("generation", "couldn't create dest directory");
						return NULL;
					}
				}
				break;
			}
			case DT_DIR: {
				Section *newSection = sectionCreate(arena, root);
				if (!sectionAddSection(root, newSection)) {
					sectionFree(newSection);
					emitError("sitegen", "couldn't add section");
					return false;
				}

				char path[128] = {0};
				strcpy(path, fullName);
				strcat(path, "/");
				newSection->path = sliceMake(fullName, pl + l + 1);

				// create the dest dir for this dir
				char *name = strrchr(fullName, '/') + 1;
				int n = strlen(destPath) + strlen(name) + 1;
				char destDir[n];
				for (int i = 0; i < n; i++) {
					destDir[i] = '\0';
				}
				strcat(destDir, destPath);
				strcat(destDir, name);
				strcat(destDir, "/");
				if (mkdir(destDir, 0755) == -1) {
					emitError("generation", "couldn't create dest directory");
					return NULL;
				}
				if (!generateRecursive(arena, newSection, path, destDir)) {
					return false;
				}
				break;
			}
		}
	}

	closedir(cdir);

	return true;
}

Section *generateSiteIR(Arena *arena, const char *path) {
	Section *root = sectionCreate(arena, NULL);
	const char *destPath = "dest/";
	if (mkdir(destPath, 0755) == -1) {
		emitError("generation", "couldn't create dest directory");
		return NULL;
	}

	if (!generateRecursive(arena, root, path, destPath)) {
		emitError("sitegen", "couldn't generate site IR");
		return NULL;
	}
	return root;
}

void generateFinalSite(Arena *arena, Section *root) {
	for (int i = 0; i < root->pagesLen; i++) {
		printf("----------\n[page ");
		slicePrint(root->pages[i]->name);
		printf("]\n");

		char *final = fillTemplate(arena, root->pages[i]);
		if (!final) {
			emitError("template", "couldn't render output");
			return;
		}
		printf("\nFinal file:\n%s\n", final);
		printf("----------\n");
		arenaFree(arena);
	}

	for (int i = 0; i < root->childrenLen; i++) {
		printf("[");
		slicePrint(root->children[i]->path);
		printf("]\n");
		generateFinalSite(arena, root->children[i]);
	}
}
