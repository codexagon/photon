#ifndef SLICE_H
#define SLICE_H

#include <stdbool.h>
#include <string.h>

#include "common.h"

// doesn't own the string, only has a view into the source
typedef struct slice {
	const char *start;
	size_t len;
} Slice;

#define sliceMake(buf, l)                                                                                              \
	(Slice) {                                                                                                            \
		.start = ((const char *)buf), .len = ((size_t)l)                                                                   \
	}

#define sliceEqual(a, b) ((a).len == (b).len) && (memcmp((a).start, (b).start, (a).len) == 0)

#define slicePrint(s) printf("%.*s", (int)((s).len), (s).start)

#endif
