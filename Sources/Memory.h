#pragma once

#include <stdlib.h>

void memory_init();
void* memory_allocate(size_t size);
void* memory_scratch_pad(size_t size);
