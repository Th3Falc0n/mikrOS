#ifndef LIST_H
#define LIST_H

struct list {
	struct list* next;
	struct list* prev;
	void* data;
};

struct list* list_new();
//struct list* list_of(void* first, ...);

struct list* list_fromarray(void** array, int length);
void** list_toarray(struct list* lst, int length);

struct list* list_head(struct list* lst);
struct list* list_first(struct list* lst);
struct list* list_last(struct list* lst);

struct list* list_insert(struct list* lst, void* data);
struct list* list_insert_list(struct list* lst, struct list* newlst);
struct list* list_insert_array(struct list* lst, void** array, int length);
struct list* list_insert_item(struct list* lst, struct list* item);

struct list* list_append(struct list* lst, void* data);
struct list* list_append_list(struct list* lst, struct list* newlst);
struct list* list_append_array(struct list* lst, void** array, int length);
struct list* list_append_item(struct list* lst, struct list* item);

struct list* list_split(struct list* lst);

void list_foreach(struct list* lst, void (*function)(struct list*));

void list_remove(struct list* lst);
void list_free(struct list* lst);

struct list* list_get(struct list* lst, int index);
struct list* list_indexof(struct list* lst, void* data);

int list_index(struct list* lst);
int list_size(struct list* lst);
int list_size_before(struct list* lst);
int list_size_after(struct list* lst);

#endif
