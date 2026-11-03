#include "str.h"

String *stringMake(Arena *arena, const char *literal, size_t size) {
	String *str = arenaAlloc(arena, sizeof(String));
	if (!str) {
		emitError("string", "couldn't allocate string");
		return NULL;
	}

	char *buf = arenaAlloc(arena, (size + 1) * sizeof(char));
	if (!buf) {
		emitError("string", "couldn't allocate buffer");
		return NULL;
	}

	buf[size] = 0;
	size_t len = 0;

	if (literal != NULL) {
		len = snprintf(buf, size + 1, "%.*s", (int)size, literal);
	}

	str->buf = buf;
	str->len = len;
	str->cap = size;
	str->idx = 0;

	return str;
}

bool stringResize(Arena *arena, String *str, size_t newSize) {
	if (newSize == str->cap) {
		emitWarning("string", "new size is equal to old size, no resizing will happen");
		return true;
	}

	char *newBuf = arenaAlloc(arena, (newSize + 1) * sizeof(char));
	if (!newBuf) {
		emitError("string", "couldn't allocate resized buffer");
		return false;
	}

	size_t len = snprintf(newBuf, str->len, "%.*s", (int)str->len, str->buf);
	str->buf = newBuf;
	str->len = len;
	str->cap = newSize;
	return true;
}

size_t stringWrite(Arena *arena, String *str, char *fmt, ...) {
	va_list args, argscpy;
	va_start(args, fmt);

	va_copy(argscpy, args);
	size_t n = vsnprintf(NULL, 0, fmt, argscpy);
	va_end(argscpy);

	if (n > str->cap) {
		if (!stringResize(arena, str, n)) {
			emitError("string", "couldn't resize string");
			return 0;
		}
	}

	n = vsprintf(str->buf, fmt, args);
	va_end(args);
	str->len = n;
	return n;
}

size_t stringAppend(Arena *arena, String *str, char *fmt, ...) {
	va_list args, argscpy;
	va_start(args, fmt);

	va_copy(argscpy, args);
	size_t n = vsnprintf(NULL, 0, fmt, argscpy);
	va_end(argscpy);

	if (str->len + n >= str->cap) {
		if (!stringResize(arena, str, str->len + n + 1)) {
			emitError("string", "couldn't resize string");
			return 0;
		}
	}

	n = vsprintf(str->buf + str->len, fmt, args);
	va_end(args);
	str->len += n;
	return n;
}

bool stringAppendChar(Arena *arena, String *str, char c) {
	if (str->len + 1 >= str->cap) {
		if (!stringResize(arena, str, str->cap * 2)) {
			emitError("string", "couldn't resize string");
			return false;
		}
	}

	str->buf[str->len++] = c;
	return true;
}
