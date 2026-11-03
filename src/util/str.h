#ifndef STR_H
#define STR_H

#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"
#include "memory/arena.h"

// doesn't own the string, only has a view into the source
typedef struct slice {
	const char *start;
	size_t len;
} Slice;

#define SLICE_NULL (Slice){0}

#define sliceMake(buf, l)                                                                                              \
	(Slice) {                                                                                                            \
		.start = ((const char *)buf), .len = ((size_t)l)                                                                   \
	}

#define sliceEqual(a, b)                                                                                               \
	(((a).start && (b).start) && ((a).len == (b).len) && (memcmp((a).start, (b).start, (a).len) == 0))

#define sliceNull(s) ((s).start == NULL || (s).len == 0)

#define slicePrint(s) printf("%.*s", (int)((s).len), (s).start)

// owns the string, terminated by \0
typedef struct string {
	char *buf;
	size_t len, cap, idx;
} String;

#define STR_NULL (String){0}

String *stringMake(Arena *arena, const char *s, size_t size);

// overwrites the original struct, you lose access to the original string
bool stringResize(Arena *arena, String *str, size_t newSize);

size_t stringWrite(Arena *arena, String *str, char *fmt, ...);
size_t stringAppend(Arena *arena, String *str, char *fmt, ...);
bool stringAppendChar(Arena *arena, String *str, char c);

#define stringAppendSlice(arena, str, slice) (stringAppend((arena), (str), "%.*s", (int)(slice).len, (slice).start))

#define stringEqual(a, b)                                                                                              \
	(((a).start && (b).start) && ((a).len == (b).len) && (memcmp((a).start, (b).start, (a).len) == 0))

#define stringNull(s) ((s).buf == NULL || (s).cap == 0)

#define stringPrint(s) printf("%s", (s).start)

#endif
