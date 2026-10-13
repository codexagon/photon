#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>
#include <unistd.h>

typedef struct arena {
	char *base;
	size_t offset;
	size_t reserved;
	size_t committed;
	size_t pageSize; // stored here to avoid using global vars
} Arena;

#define arenaFree(arena) (arena)->offset = 0

void getPageSize(Arena *arena);
uintptr_t alignForward(uintptr_t ptr, size_t align);

Arena *arenaCreate(size_t reserveSize);
void *arenaAlloc(Arena *arena, size_t size);
void arenaRelease(Arena *arena);

#endif
