#include <dirent.h>
#include <errno.h>
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

// handle EEXIST error separately so that we don't have to remove dest dir everytime
bool createDir(const char *path) {
	if (mkdir(path, 0755) == 0) {
		return true;
	}

	if (errno != EEXIST) {
		return false;
	}

	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
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
				char *file = readFile(arena, fullName);
				if (!file) {
					emitError("sitegen", "couldn't read file");
					closedir(cdir);
					return false;
				}

				Page *newPage = pageCreate(arena, root);
				if (!sectionAddPage(root, newPage)) {
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

				newPage->metadata = parseMetadata(arena, file);
				newPage->content = tokenizeBlocks(arena, file);

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
				if (!sectionAddSection(root, newSection)) {
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

Section *generateSiteIR(Arena *arena, const char *path) {
	Section *root = sectionCreate(arena, NULL);
	root->path = sliceMake("content", 7);
	const char *destPath = "dest/";
	if (!createDir(destPath)) {
		emitError("generation", "couldn't create dest directory");
		return NULL;
	}

	if (!generateRecursive(arena, root, path, destPath)) {
		emitError("sitegen", "couldn't generate site IR");
		return NULL;
	}
	return root;
}

bool getDestFilePath(char *dest, char *src) {
	char *start = strchr(src, '/') + 1;
	char *name = strrchr(src, '/') + 1;

	if (!start || !name) {
		return false;
	}

	// assumes dest is zeroed out initially
	strcat(dest, "dest/");
	int idx = strlen("dest/");
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

void generateFinalSite(Arena *arena, Section *root) {
	for (int i = 0; i < root->pagesLen; i++) {
		Page *curPage = root->pages[i];

		int n = snprintf(NULL, 0, "%s/%.*s", curPage->parent->path.start, (int)curPage->name.len, curPage->name.start) + 1;
		char src[n];
		snprintf(src, sizeof(src), "%s/%.*s", curPage->parent->path.start, (int)curPage->name.len, curPage->name.start);

		char dest[128] = {0};
		if (!getDestFilePath(dest, src)) {
			emitError("generation", "couldn't get destination filepath");
			return;
		}

		printf("Generating %s\n", dest);

		char *final = fillTemplate(arena, root->pages[i]);
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

		if (fwrite(final, strlen(final), 1, destFile) != 1) {
			emitError("generation", "couldn't write dest file");
			return;
		}

		fclose(destFile);
		arenaFree(arena);
	}

	for (int i = 0; i < root->childrenLen; i++) {
		generateFinalSite(arena, root->children[i]);
	}
}
