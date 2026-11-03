#include <dirent.h>

#include "common.h"
#include "sitegen.h"
#include "template/template.h"
#include "util/util.h"

PageNode *createPageNode(Arena *arena, Page *page) {
	PageNode *pnode = arenaAlloc(arena, sizeof(PageNode));
	if (!pnode) {
		emitError("sitegen", "couldn't create page node");
		return NULL;
	}

	pnode->page = page;
	pnode->next = NULL;
	return pnode;
}

SectionNode *createSectionNode(Arena *arena, Section *section) {
	SectionNode *snode = arenaAlloc(arena, sizeof(SectionNode));
	if (!snode) {
		emitError("sitegen", "couldn't create section node");
		return NULL;
	}

	snode->section = section;
	snode->next = NULL;
	return snode;
}

Page *pageCreate(Arena *arena, Section *parent) {
	Page *page = arenaAlloc(arena, sizeof(Page));
	if (!page) {
		emitError("sitegen", "failed to create page");
		return NULL;
	}
	page->name = SLICE_NULL;
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
	section->path = SLICE_NULL;

	section->children = NULL;
	section->pages = NULL;

	section->parent = parent;
	return section;
}

bool sectionAddPage(Arena *arena, Section *section, Page *new) {
	PageNode *newPage = createPageNode(arena, new);
	if (!newPage) {
		return false;
	}

	newPage->next = section->pages;
	section->pages = newPage;

	return true;
}

bool sectionAddSection(Arena *arena, Section *section, Section *new) {
	SectionNode *newSection = createSectionNode(arena, new);
	if (!newSection) {
		return false;
	}

	newSection->next = section->children;
	section->children = newSection;

	return true;
}

// builds site IR and generates destination dir structure too
bool generateRecursive(Arena *arena, Section *root, const char *path, const char *destPath) {
	DIR *cdir = opendir(path);
	if (!cdir) {
		emitError("sitegen", "failed to open directory");
		closedir(cdir);
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
		int fullNameSize = snprintf(NULL, 0, "%s%s", path, entry->d_name) + 1;
		char *fullName = arenaAlloc(arena, fullNameSize);
		if (!fullName) {
			emitError("sitegen", "couldn't allocate full path string");
			closedir(cdir);
			return false;
		}

		snprintf(fullName, fullNameSize, "%s%s", path, entry->d_name);

		switch (entry->d_type) {
			case DT_REG: {
				String *file = readFile(arena, fullName);
				if (!file) {
					emitError("sitegen", "couldn't read file");
					closedir(cdir);
					return false;
				}

				Page *newPage = pageCreate(arena, root);
				if (!sectionAddPage(arena, root, newPage)) {
					emitError("sitegen", "couldn't add page");
					closedir(cdir);
					return false;
				}

				char *nameStart = strrchr(fullName, '/') + 1;
				if (!nameStart) {
					emitError("sitegen", "couldn't find last '/'");
					closedir(cdir);
					return false;
				}
				newPage->name = sliceMake(nameStart, l - 3);

				// create the dest dir for this file if its name isn't root.md
				if (!sliceEqual(newPage->name, sliceMake("root", 4))) {
					int n = snprintf(NULL, 0, "%s%.*s", destPath, (int)newPage->name.len, newPage->name.start) + 1;
					char destDir[n];
					snprintf(destDir, n, "%s%.*s", destPath, (int)newPage->name.len, newPage->name.start);

					if (!createDir(destDir)) {
						emitError("generation", "couldn't create dest directory");
						closedir(cdir);
						return false;
					}
				}
				break;
			}
			case DT_DIR: {
				Section *newSection = sectionCreate(arena, root);
				if (!sectionAddSection(arena, root, newSection)) {
					emitError("sitegen", "couldn't add section");
					closedir(cdir);
					return false;
				}

				newSection->path = sliceMake(fullName, pl + l + 1);

				// create the dest dir for this dir
				char *name = strrchr(fullName, '/') + 1;
				int n = snprintf(NULL, 0, "%s%s/", destPath, name) + 1;
				char destDir[n];
				snprintf(destDir, n, "%s%s/", destPath, name);

				if (!createDir(destDir)) {
					emitError("generation", "couldn't create dest directory");
					closedir(cdir);
					return false;
				}

				n = snprintf(NULL, 0, "%s/", fullName) + 1;
				char path[n];
				snprintf(path, sizeof(path), "%s/", fullName);
				if (!generateRecursive(arena, newSection, path, destDir)) {
					closedir(cdir);
					return false;
				}
				break;
			}
		}
	}

	closedir(cdir);

	return true;
}

Section *generateSiteIR(Arena *arena, const char *path, char *destDirName) {
	Section *root = sectionCreate(arena, NULL);
	root->path = sliceMake("content", 7);
	if (!createDir(destDirName)) {
		emitError("sitegen", "couldn't create dest directory");
		return NULL;
	}

	if (!generateRecursive(arena, root, path, destDirName)) {
		emitError("sitegen", "couldn't generate site IR");
		return NULL;
	}
	return root;
}

bool parseSite(Arena *arena, Section *root) {
	for (PageNode *p = root->pages; p != NULL; p = p->next) {
		Page *page = p->page;
		int pathLen = snprintf(NULL, 0, "%.*s/%.*s.md", (int)page->parent->path.len, page->parent->path.start,
		                       (int)page->name.len, page->name.start) +
		              1;
		char path[pathLen];
		path[pathLen - 1] = 0;
		snprintf(path, pathLen, "%.*s/%.*s.md", (int)page->parent->path.len, page->parent->path.start, (int)page->name.len,
		         page->name.start);

		String *file = readFile(arena, path);
		if (!file) {
			emitError("parser", "couldn't read file");
			return false;
		}

		if (!parseMarkdown(arena, file, &page->metadata, &page->content)) {
			emitError("parser", "couldn't parse file");
			return false;
		}
	}

	for (SectionNode *s = root->children; s != NULL; s = s->next) {
		if (!parseSite(arena, s->section)) {
			emitError("parser", "couldn't parse section");
			return false;
		}
	}

	return true;
}

bool getDestFilePath(char *dest, char *src, char *destDirName) {
	char *start = strchr(src, '/') + 1;
	char *name = strrchr(src, '/') + 1;

	if (!start || !name) {
		return false;
	}

	// assumes dest is zeroed out initially
	strcat(dest, destDirName);
	int idx = strlen(destDirName);
	char *s = start;
	while (s < name) {
		dest[idx++] = *s++;
	}

	if (strcmp(name, "root") != 0) {
		strcat(dest, name);
		strcat(dest, "/");
	}

	strcat(dest, "index.html");
	return true;
}

void generateFinalSite(Arena *arena, Section *root, char *destDirName) {
	for (PageNode *p = root->pages; p != NULL; p = p->next) {
		Page *curPage = p->page;

		int n = snprintf(NULL, 0, "%s/%.*s", curPage->parent->path.start, (int)curPage->name.len, curPage->name.start) + 1;
		char src[n];
		snprintf(src, sizeof(src), "%s/%.*s", curPage->parent->path.start, (int)curPage->name.len, curPage->name.start);

		char dest[128] = {0};
		if (!getDestFilePath(dest, src, destDirName)) {
			emitError("generation", "couldn't get destination filepath");
			return;
		}

		emitInfo(NULL, "Generating %s", dest);

		String *final = fillTemplate(arena, curPage);
		if (!final) {
			emitError("template", "couldn't render output");
			return;
		}

		// write rendered files to disk
		FILE *destFile = fopen(dest, "w");
		if (!destFile) {
			emitError("generation", "couldn't create dest file");
			return;
		}

		if (fwrite(final->buf, final->len, 1, destFile) != 1) {
			emitError("generation", "couldn't write dest file");
			return;
		}

		fclose(destFile);
		arenaFree(arena);
	}

	for (SectionNode *s = root->children; s != NULL; s = s->next) {
		generateFinalSite(arena, s->section, destDirName);
	}
}
