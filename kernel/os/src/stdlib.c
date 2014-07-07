#include "stdlib.h"

struct memory_node* first_unused = 0;
struct memory_node* first_used   = 0;
struct memory_node* first_free   = 0;

static void remove_from_list(struct memory_node** root, struct memory_node* element) {
  struct memory_node* last = 0;
  struct memory_node* cur  = *root;
  
  while(cur != 0) {
    if(cur == element) {
      if(last == 0) {
        *root = cur->next;
        return;
      }
      else
      {
        last->next = cur->next;
        return;
      }
    }
    last = cur;
    cur = cur->next;
  }
}

static void append_to_list(struct memory_node** root, struct memory_node* element) {
  element->next = *root;
  *root = element;
}

static struct memory_node* pop_from_list(struct memory_node** root) {
  if(*root == 0) return 0;
  struct memory_node* pop = *root;
  remove_from_list(root, pop);
  return pop;
}

static void allocate_unused_nodes() {
  struct memory_node* new_nodes = vmm_alloc_cont(1);
  memset(new_nodes, 0, PAGESIZE);
 
  for(uint32_t i = 1; i < (PAGESIZE / sizeof(struct memory_node)); i++) {
    append_to_list(&first_unused, &(new_nodes[i]));
  }
}

static struct memory_node* pop_unused_node() {
  struct memory_node* ret = pop_from_list(&first_unused);
  
  while(ret == 0) {
    allocate_unused_nodes();
    ret = pop_from_list(&first_unused); 
  }
  
  return ret;
}

static void merge_into_frees(struct memory_node* tf) {
  remove_from_list(&first_used, tf);
  
  struct memory_node* last;
  struct memory_node* cur;
  
  editedList:
  last = 0;
  cur  = first_free;
  
  while(cur != 0) {
    if(cur->address + cur->size == tf->address) {
      tf->address = cur->address;
      remove_from_list(&first_free, cur);
      goto editedList;
    }
    
    if(cur->address == tf->address + tf->size) {
      tf->size += cur->size;
      remove_from_list(&first_free, cur);
      goto editedList;
    }
    
    last = cur;
    cur = cur->next;
  }
  
  append_to_list(&first_free, tf);
}

void* malloc(size_t size) {
  struct memory_node* last = 0;
  struct memory_node* cur  = first_free;
    
  while(cur != 0) {
    if(cur->size >= size) {
      break;
    }
    last = cur;
    cur = cur->next;
  }

  if(cur == 0) {
    uint32_t pgs = size / PAGESIZE;
    
    if((size % PAGESIZE) != 0)
      pgs++;
    
    void* addr = vmm_alloc_cont(pgs);
    
    struct memory_node* fill = pop_unused_node();
    
    fill->address = (uint32_t)addr;
    fill->size    = (uint32_t)size;
    
    if(pgs * PAGESIZE > size) {
      struct memory_node* free = pop_unused_node();
      
      free->address = fill->address + fill->size;
      free->size = size - pgs * PAGESIZE;
      
      append_to_list(&first_free, free);
    }    
    
    append_to_list(&first_used, fill);
    
    return (void*) fill->address;
  }
  else
  {
    uint32_t freesize = size - cur->size;
      
    cur->size = size;
    
    remove_from_list(&first_free, cur);
    append_to_list(&first_used, cur);
    
    if(free > 0) {
      struct memory_node* free = pop_unused_node();
      
      free->address = cur->address + cur->size;
      free->size = freesize;
      
      append_to_list(&first_free, free);
    }
    
    return (void*)cur->address;
  }
}

void* calloc(size_t num, size_t size) {
  size_t gsize = num * size;
  void* p = malloc(gsize);
  
  if(p != 0) {
    memset(p, 0, gsize);
  }
  
  return p;
}

void* realloc(void* ptr, size_t size) {
  struct memory_node* last = 0;
  struct memory_node* cur  = first_used;
    
  while(cur != 0) {
    if(cur->address == (uint32_t)ptr) {
      if(size == 0) {
        free(ptr);
        return 0;
      }
      else
      {
        void* new = malloc(size);
        memcpy(new, (void*)cur->address, cur->size);
        free((void*)cur->address);
        
        return new;
      }  
    }
    last = cur;
    cur = cur->next;
  }
  return 0;
}

void free(void* ptr) {
  struct memory_node* last = 0;
  struct memory_node* cur  = first_used;
    
  while(cur != 0) {
    if(cur->address == (uint32_t)ptr) {
      merge_into_frees(cur);
      break;
    }
    last = cur;
    cur = cur->next;
  }
}
