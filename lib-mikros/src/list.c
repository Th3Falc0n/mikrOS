#include "list.h"
#include "stdlib.h"
//#include "stdarg.h"

struct list* list_new() {
	struct list* lst = malloc(sizeof(struct list));

	lst->next = 0;
	lst->prev = 0;
	lst->data = 0;

	return lst;
}

/*struct list* list_of(void* first, ...) {
	va_list elements;
	va_start(elements, first);

	struct list* newlst = list_new();

	void* element = first;
	do {
		list_append(newlst, element);
	} while ((element = va_arg(elements, void*)));

	va_end(elements);

	return newlst;
}*/

struct list* list_fromarray(void** array, int length) {
	struct list* newlst = list_new();

	list_append_array(newlst, array, length);

	return newlst;
}

void** list_toarray(struct list* lst, int length) {
	if (!lst->prev)
		lst = lst->next;

	if (!lst)
		return 0;

	if (length < 0)
		length = list_size_after(lst);

	void** array = malloc(sizeof(void*) * (length + 1));

	for (int i = 0; i < length; i++) {
		array[i] = lst->data;
		lst = lst->next;
	}
	array[length] = 0;

	return array;
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

struct list* list_insert_list(struct list* lst, struct list* newlst) {
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

	return newfirst;
}

struct list* list_insert_array(struct list* lst, void** array, int length) {
	struct list* newlst = list_new();

	for (int i = 0; (i < length || (length < 0 && array[i])); i++) {
		list_append(newlst, array[i]);
	}

	return list_insert_list(lst, newlst);
}

struct list* list_insert_item(struct list* lst, struct list* item) {
	list_remove(item);

	item->next = lst->next;
	item->prev = lst;

	if (lst->next)
		lst->next->prev = item;
	lst->next = item;

	return item;
}

struct list* list_append(struct list* lst, void* data) {
	return list_insert(list_last(lst), data);
}

struct list* list_append_list(struct list* lst, struct list* newlst) {
	return list_insert_list(list_last(lst), newlst);
}

struct list* list_append_array(struct list* lst, void** array, int length) {
	return list_insert_array(list_last(lst), array, length);
}

struct list* list_append_item(struct list* lst, struct list* item) {
	return list_insert_item(list_last(lst), item);
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
	lst = list_head(lst);
	while (lst->next) {
		lst = lst->next;
		function(lst);
	}
}

void list_free(struct list* lst) {
	if (!lst->prev) {
		list_foreach(lst, (void*) free);
	} else {
		list_remove(lst);
	}
	free(lst);
}

struct list* list_get(struct list* lst, int index) {
	lst = list_first(lst);

	for (int i = 0; i < index; i++) {
		if (!lst)
			return 0;
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
	while ((lst = lst->prev))
		index++;
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
