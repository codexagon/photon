#include "template.h"
#include "render/render.h"
#include "util/util.h"

Slice getMetadataValue(Page *page, char *key, size_t keyLen) {
	int idx = 0;
	while (idx < page->metadata->len &&
	       (keyLen != page->metadata->keys[idx].len || strncmp(page->metadata->keys[idx].start, key, keyLen) != 0)) {
		idx++;
	}

	if (idx == page->metadata->len) {
		emitError("template", "couldn't find metadata field");
		return SLICE_NULL;
	}

	return page->metadata->values[idx];
}

int handlePlaceholder(Slice placeholder, Arena *arena, Page *page, char *out) {
	if (sliceEqual(placeholder, sliceMake("title", 5))) {
		Slice value = getMetadataValue(page, "title", 5);
		if (sliceEmpty(value)) {
			return -1;
		}
		strncat(out, value.start, value.len);
		return value.len;
	} else if (sliceEqual(placeholder, sliceMake("date", 4))) {
		Slice value = getMetadataValue(page, "date", 4);
		if (sliceEmpty(value)) {
			return -1;
		}
		strncat(out, value.start, value.len);
		return value.len;
	} else if (sliceEqual(placeholder, sliceMake("tags", 4))) {
		Slice value = getMetadataValue(page, "tags", 4);
		if (sliceEmpty(value)) {
			return -1;
		}
		strncat(out, value.start, value.len);
		return value.len;
	} else if (sliceEqual(placeholder, sliceMake("content", 7))) {
		char *renderedHTML = mdToHTML(arena, page->content);
		if (!renderedHTML) {
			emitError("template", "couldn't write body");
			return -1;
		}
		size_t l = strlen(renderedHTML);
		strncat(out, renderedHTML, l);
		return l;
	}

	return -1;
}

char *fillTemplate(Arena *arena, Page *page) {
	Slice template = getMetadataValue(page, "template", 8);
	if (sliceEmpty(template)) {
		return NULL;
	}

	int n = strlen("templates/.txt") + template.len + 1;
	char templatePath[n];
	memset(templatePath, 0, n);

	snprintf(templatePath, n, "templates/%.*s.txt", (int)template.len, template.start);

	char *templateFile = readFile(arena, templatePath);
	if (!templateFile) {
		emitError("template", "couldn't load template file");
		return NULL;
	}

	// since i don't know what the final size is gonna be
	char *filledTemplate = arenaAlloc(arena, 3 * strlen(templateFile));
	if (!filledTemplate) {
		emitError("template", "couldn't allocate output buffer");
		return NULL;
	}

	char *in = templateFile;
	char *out = filledTemplate;
	while (*in != '\0') {
		if (*in == '{') {
			in += 2;
			char *end = strchr(in, '}');
			Slice placeholder = sliceMake(in, (end - in));
			int written = handlePlaceholder(placeholder, arena, page, out);
			if (written == -1) {
				emitError("template", "couldn't fill in placeholder");
				return NULL;
			}
			in = end + 2;
			out += written;
		} else {
			*out++ = *in++;
		}
	}
	*out = '\0';

	return filledTemplate;
}
