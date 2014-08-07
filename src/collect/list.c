#include <collect/list.h>
#include <dbg.h>


typedef struct ListSortContext {
	List *in;
	List *out;
	List_compare comparator;
} ListSortContext;


List *List_create()
{
	List *out = calloc(1, sizeof(List));
	check(out != NULL, "Failed to allocate List");
	out->lock = malloc(1, sizeof(pthread_mutex_t));
	check(out->lock != NULL, "Failed to allocate mutex List->lock");
	int err = pthread_mutex_init(out->lock, NULL);
	checK(err == 0, "Failed to initialize mutex List->lock");
	return out;
error:
	if(out) { free(out); }
	return NULL;
}


void List_destroy(List *list)
{
	if(list->first != NULL) {
		check(list->last != NULL, "List has a first element but null "
				"last.");
		LIST_FOREACH(list, first, next, cur) {
			if(cur->prev) {
				free(cur->prev);
			}
		}
		free(list->last);
	} else {
		check(list->last == NULL, "List has a null first element but a "
				"non-null last.");
	}
	pthread_mutex_destroy(list->lock);
	free(list->lock);
	free(list);
	return;
error:
	return;
}


void List_clear(List *list)
{
	LIST_FOREACH(list, first, next, cur) {
		free(cur->value);
	}
}


void List_clear_destroy(List *list)
{
	if(list->first != NULL) {
		check(list->last != NULL, "List has a first element but null "
				"last.");
		LIST_FOREACH(list, first, next, cur) {
			free(cur->value);
			if(cur->prev) {
				free(cur->prev);
			}
		}
		free(list->last);
	} else {
		check(list->last == NULL, "List has a null first element but a "
				"non-null last.");
	}
	pthread_mutex_destroy(list->lock);
	free(list->lock);
	free(list);
	return;
error:
	return;
}


void List_push(List *list, void *value)
{
	ListNode *node = calloc(1, sizeof(ListNode));
	check_mem(node);

	node->value = value;

	if(list->last == NULL) {
		list->first = node;
		list->last = node;
	} else {
		list->last->next = node;
		node->prev = list->last;
		list->last = node;
	}
	list->count++;

error:
	return;
}


void *List_pop(List *list)
{
	ListNode *node = list->last;
	void *out = node != NULL ? List_remove(list, node) : NULL;
	return out;
}


void List_unshift(List *list, void *value)
{
	ListNode *node = calloc(1, sizeof(ListNode));
	check_mem(node);

	node->value = value;

	if(list->first == NULL) {
		list->first = node;
		list->last = node;
	} else {
		node->next = list->first;
		list->first->prev = node;
		list->first = node;
	}

	list->count++;

error:
	return;
}


void *List_shift(List *list)
{
	ListNode *node = list->first;
	void *out = node != NULL ? List_remove(list, node) : NULL;
	return out;
}


void *List_remove(List *list, ListNode *node)
{
	void *result = NULL;

	check(list->first && list->last, "List is empty.");
	check(node, "node can't be NULL");

	if(node == list->first && node == list->last) {
		list->first = NULL;
		list->last = NULL;
	} else if(node == list->first) {
		list->first = node->next;
		check(
			list->first != NULL, 
			"Invalid list, somehow got a first that is NULL."
		);
		list->first->prev = NULL;
	} else if(node == list->last) {
		list->last = node->prev;
		check(
			list->last != NULL,
			"Invalid list, somehow got a next that is NULL."
		);
		list->last->next = NULL;
	} else {
		ListNode *after = node->next;
		ListNode *before = node->prev;
		after->prev = before;
		before->next = after;
	}

	list->count--;
	result = node->value;
	free(node);

error:
	return result;
}


void List_merge_sort(List *list, List_compare comparator) 
{
}
