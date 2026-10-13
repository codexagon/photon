#include <assert.h>
#include <sys/mman.h>

#include "arena.h"
#include "common.h"

void getPageSize(Arena *a) {
	a->pageSize = (size_t)sysconf(_SC_PAGESIZE);
}

uintptr_t alignForward(uintptr_t ptr, size_t align) {
	uintptr_t p, a, mod; // a is a power of 2

	p = ptr;
	a = (uintptr_t)align;

	assert((a & (a - 1)) == 0);
	// this is same as p % a but since a is a power of 2, this is faster
	mod = p & (a - 1);
	if (mod != 0) {
		p += a - mod;
	}

	return p;
}

Arena *arenaCreate(size_t reserveSize) {
	Arena *arena = calloc(1, sizeof(Arena));
	getPageSize(arena);
	reserveSize = (size_t)alignForward((uintptr_t)reserveSize, arena->pageSize);

	void *block = mmap(NULL, reserveSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (block == MAP_FAILED) {
		emitError("mmap failed, couldn't create arena\n");
		return NULL;
	}

	arena->base = (char *)block;
	arena->offset = 0;
	arena->reserved = reserveSize;
	arena->committed = 0;

	return arena;
}

void *arenaAlloc(Arena *arena, size_t size) {
	if (!arena || size == 0) {
		emitError("invalid size or arena passed");
		return NULL;
	}

	size_t newOffset = arena->offset + size;
	if (newOffset > arena->reserved) {
		emitError("out of memory");
		return NULL;
	}

	if (newOffset > arena->committed) {
		size_t commitTarget = alignForward(newOffset, arena->pageSize);
		if (commitTarget > arena->reserved) {
			commitTarget = arena->reserved;
		}

		size_t commitSize = commitTarget - arena->committed;
		void *commitStart = arena->base + arena->committed;

		if (mprotect(commitStart, commitSize, PROT_READ | PROT_WRITE) != 0) {
			emitError("memory commit failed");
			return NULL;
		}

		arena->committed = commitTarget;
	}

	void *memory = arena->base + arena->offset;
	arena->offset = newOffset;
	memset(memory, 0, size);

	return memory;
}

void arenaRelease(Arena *arena) {
	arenaFree(arena);
	munmap(arena->base, arena->reserved);
	free(arena);
}
