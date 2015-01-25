#include "list.h"
#include "stdlib.h"

struct list* list_new() {
	struct list* lst = malloc(sizeof(struct list));

	lst->next = 0;
	lst->prev = 0;
	lst->data = 0;

	return lst;
}

struct list* list_head(struct list* lst) {
	while (lst->prev)
		lst = lst->prev;
	return lst;
}

struct list* list_first(struct list* lst) {
	return list_head(lst)->next;
}

struct list* list_last(struct list* lst) {
	while (lst->next)
		lst = lst->next;
	return lst;
}

struct list* list_insert(struct list* lst, void* data) {
	struct list* new = malloc(sizeof(struct list));

	new->next = lst->next;
	new->prev = lst;
	new->data = data;

	if (lst->next)
		lst->next->prev = new;
	lst->next = new;

	return new;
}

void list_insert_list(struct list* lst, struct list* newlst) {
	struct list* newhead = list_head(newlst);
	struct list* newfirst = newhead->next;
	newhead->next = 0;
	list_free(newhead);

	if (newfirst) {
		struct list* newlast = list_last(newfirst);

		newfirst->prev = lst;
		newlast->next = lst->next;

		lst->next = newfirst;
		if (lst->next)
			lst->next->prev = newlast;
	}
}

struct list* list_append(struct list* lst, void* data) {
	return list_insert(list_last(lst), data);
}

void list_append_list(struct list* lst, struct list* newlst) {
	return list_insert_list(list_last(lst), newlst);
}

struct list* list_split(struct list* lst) {
	if (!lst->prev)
		return lst;

	struct list* newlst = list_new();

	lst->prev->next = 0;
	lst->prev = 0;

	list_append_list(newlst, lst);

	return newlst;
}

void list_remove(struct list* lst) {
	if (!lst->prev)
		return;

	lst->prev->next = lst->next;
	if (lst->next)
		lst->next->prev = lst->prev;
}

void list_foreach(struct list* lst, void (*function)(struct list*)) {
	struct list* last;

	lst = list_head(lst);
	while (lst->next) {
		last = lst;
		lst = lst->next;
		function(last);
	}
}

void list_free(struct list* lst) {
	if (!lst->prev && lst->next) {
		list_foreach(lst->next, (void*) free);
	} else {
		list_remove(lst);
	}
	free(lst);
}

void** list_toarray(struct list* lst, int length) {
	if (length < 0)
		length = list_size_after(lst);

	void** array = malloc(sizeof(void*) * (length + 1));

	for (int i = 0; i < length; i++) {
		array[i] = lst;
		lst = lst->next;
	}
	array[length] = 0;

	return array;
}

struct list* list_get(struct list* lst, int index) {
	lst = list_head(lst);

	for (int i = -1; i < index; i++) {
		lst = lst->next;
	}

	return lst;
}

struct list* list_indexof(struct list* lst, void* data) {
	lst = list_head(lst);

	while (lst->next) {
		lst = lst->next;
		if (lst->data == data)
			return lst;
	}

	return 0;
}

int list_index(struct list* lst) {
	int index = 0;

	while (lst->prev) {
		lst = lst->prev;
		index++;
	}

	return index - 1;
}

int list_size(struct list* lst) {
	return list_size_before(lst) + list_size_after(lst);
}

int list_size_before(struct list* lst) {
	int size = 0;
	while ((lst = lst->prev))
		if (lst->prev)
			size++;
	return size;
}

int list_size_after(struct list* lst) {
	int size = 1;
	while ((lst = lst->next))
		size++;
	return size;
}
