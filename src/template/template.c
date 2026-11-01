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

size_t handlePlaceholder(Slice placeholder, Arena *arena, Page *page, String *out) {
	if (sliceEqual(placeholder, sliceMake("title", 5))) {
		Slice value = getMetadataValue(page, "title", 5);
		if (sliceNull(value)) {
			return -1;
		}
		stringAppend(arena, out, "%.*s", (int)value.len, value.start);
		return value.len;
	} else if (sliceEqual(placeholder, sliceMake("date", 4))) {
		Slice value = getMetadataValue(page, "date", 4);
		if (sliceNull(value)) {
			return -1;
		}
		stringAppend(arena, out, "%.*s", (int)value.len, value.start);
		return value.len;
	} else if (sliceEqual(placeholder, sliceMake("tags", 4))) {
		Slice value = getMetadataValue(page, "tags", 4);
		if (sliceNull(value)) {
			return -1;
		}
		stringAppend(arena, out, "%.*s", (int)value.len, value.start);
		return value.len;
	} else if (sliceEqual(placeholder, sliceMake("content", 7))) {
		String *renderedHTML = mdToHTML(arena, page->content);
		if (!renderedHTML) {
			emitError("template", "couldn't write body");
			return -1;
		}
		stringAppend(arena, out, "%.*s", (int)renderedHTML->len, renderedHTML->buf);
		return renderedHTML->len;
	}

	return -1;
}

String *fillTemplate(Arena *arena, Page *page) {
	Slice template = getMetadataValue(page, "template", 8);
	if (sliceNull(template)) {
		return NULL;
	}

	int n = strlen("templates/.txt") + template.len + 1;
	char templatePath[n];
	memset(templatePath, 0, n);

	snprintf(templatePath, n, "templates/%.*s.txt", (int)template.len, template.start);

	String *templateFile = readFile(arena, templatePath);
	if (!templateFile) {
		emitError("template", "couldn't load template file");
		return NULL;
	}

	// since i don't know what the final size is gonna be
	String *filledTemplate = stringMake(arena, NULL, 3 * templateFile->len);
	if (!filledTemplate) {
		emitError("template", "couldn't allocate output buffer");
		return NULL;
	}

	templateFile->idx = 0;
	while (templateFile->idx < templateFile->len) {
		if (strncmp(templateFile->buf + templateFile->idx, "{{", 2) == 0) {
			templateFile->idx += 2;

			char *placeholderStart = templateFile->buf + templateFile->idx;
			size_t placeholderLen = 0;
			while (strncmp(templateFile->buf + templateFile->idx, "}}", 2) != 0) {
				templateFile->idx++;
				placeholderLen++;
			}

			Slice placeholder = sliceMake(placeholderStart, placeholderLen);

			int written = handlePlaceholder(placeholder, arena, page, filledTemplate);
			if (written == -1) {
				emitError("template", "couldn't fill in placeholder");
				return NULL;
			}

			templateFile->idx += 2;
		} else {
			filledTemplate->buf[filledTemplate->len++] = templateFile->buf[templateFile->idx++];
		}
	}

	return filledTemplate;
}
