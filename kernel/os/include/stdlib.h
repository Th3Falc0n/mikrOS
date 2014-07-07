#ifndef STDLIB_H
#define STDLIB_H

#include "string.h"
#include "vmm.h"

#define PAGESIZE 4096

struct memory_node {
  uint32_t            size;
  uint32_t            address;
  struct memory_node* next;
};

void* malloc  (size_t size);
void* calloc  (size_t num, size_t size);
void* realloc (void* ptr, size_t size);
void  free    (void* ptr);
	
#endif
