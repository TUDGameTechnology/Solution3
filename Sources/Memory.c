#include <assert.h>
#include <stdint.h>

#include "Memory.h"

#define MEMORY_SIZE 10 * 1024 * 1024
#define SCRATCH_PAD_SIZE 1 * 1024 * 1024

uint8_t* memory;
size_t index;

void memory_init() {
	memory = malloc(sizeof(uint8_t) * MEMORY_SIZE);
	index = SCRATCH_PAD_SIZE;
}

void* memory_scratch_pad(size_t size) {
	assert(size < SCRATCH_PAD_SIZE);
	return memory;
}

void* memory_allocate(size_t size) {
	assert(index + size < MEMORY_SIZE);
	void* data = &memory[index];
	index += size;
	return data;
}
