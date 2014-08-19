/*
 * Linked List data structure.  Includes threaded sorting.
 * Copyright (C) 2014 Axel Magnuson <axelmagn@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <collect/list.h>
#include <dbg.h>

#define DEFAULT_PTHREAD_LIMIT 50;


typedef struct ListSortContext {
	List *list;
	ListNode *start;
	int extent;
	int max_threads;
	List_compare comparator;
	int *thread_count;
	pthread_mutex_t *lock;
} ListSortContext;


/// Allocate a new list from the heap.
List *List_create()
{
	List *out = calloc(1, sizeof(List));
	check(out != NULL, "Failed to allocate List");
	out->lock = calloc(1, sizeof(pthread_mutex_t));
	check(out->lock != NULL, "Failed to allocate mutex List->lock");
	int err = pthread_mutex_init(out->lock, NULL);
	check(err == 0, "Failed to initialize mutex List->lock");
	return out;
error:
	if(out->lock) { free(out->lock); }
	if(out) { free(out); }
	return NULL;
}


/// Free a list, as well as any nodes belonging to it.
/**
 * List_destroy frees list resources, but does not free the values of its
 * nodes. Refer to List_clear and List_clear_destroy for freeing node values.
 */
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


/// Free all values contained by the list.
/**
 * List_clear frees the values contained by a list, but does not free the list
 * or its nodes. Refer to List_destroy and list_clear_destroy for freeing list 
 * structures.
 */
void List_clear(List *list)
{
	LIST_FOREACH(list, first, next, cur) {
		free(cur->value);
	}
}


/// Free a list, its nodes, and any values it contains.
/**
 * List_clear_destroy frees a list structure, as well as the values it
 * contains.  It is equivalent to calling List_clear followed by List_destroy.
 */
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




/// retrieve the nodel located at an index
ListNode *List_get_node(List *list, int index) {
	ListNode *out = NULL;

	// validate input
	check(list, "Received null pointer for list.");
	check(index < list->count, "List size is %d.  Index %d out of bounds.", 
			list->count, index);

	// seek the node in question.  Seek from whichever side is closest to
	// minimize time
	int i;
	if(index <= list->count / 2) { 
		// seek forward
		out = list->first;
		for(i = 0; i < index; i++) {
			out = out->next;
		}
	} else { 
		// seek backward
		out = list->last;
		for(i = list->count - 1; i > index; i--) {
			out = out->prev;
		}
	}
error:
	return out;
}

/// retrieve the value stored at an index
void *List_get(List *list, int index) {
	return List_get_node(list, index)->value;
}


/// push a new value onto the end of the list.
void List_push(List *list, void *value)
{
	// allocate a new node
	ListNode *node = calloc(1, sizeof(ListNode));
	check_mem(node);

	// store the value in the new node
	node->value = value;

	// append the node at the end of the list
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


/// remove and return the end of the list.
void *List_pop(List *list)
{
	ListNode *node = list->last;
	void *out = node != NULL ? List_remove(list, node) : NULL;
	return out;
}


/// push a new value onto the beginning of the list.
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


/// remove and return the beginning of the list.
void *List_shift(List *list)
{
	ListNode *node = list->first;
	void *out = node != NULL ? List_remove(list, node) : NULL;
	return out;
}


/// remove and return a specified node from the list.
void *List_remove(List *list, ListNode *node)
{
	void *result = NULL;

	// check that we can actually remove the node
	check(list->first && list->last, "List is empty.");
	check(node, "node can't be NULL");

	// unlink node from list
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


	// adjust count
	list->count--;

	// store the value and delete the orphan node
	result = node->value;
	free(node);

error:
	return result;
}

/// Create a context for sort subroutines to share
ListSortContext *ListSortContext_create( List *list, ListNode *start, 
		int extent, int max_threads, List_compare comparator)
{
	ListSortContext *out = calloc(1, sizeof(ListSortContext));
	check(out != NULL, "Failed to allocate ListSortContext");
	out->lock = calloc(1, sizeof(pthread_mutex_t));
	check(out->lock != NULL, "Failed to allocate mutex");
	int err = pthread_mutex_init(out->lock, NULL);
	check(err == 0, "Failed to initialize mutex");
	out->thread_count = calloc(1, sizeof(int));
	check(out->thread_count != NULL, "Failed to allocate thread_count");
	*(out->thread_count) = 0;
	out->list = list;
	out->start = start;
	out->extent = extent;
	out->max_threads = max_threads;
	out->comparator = comparator;
	return out;
error:
	if(out->thread_count) { free(out->thread_count); }
	if(out->lock) { free(out->lock); }
	if(out) { free(out); }
	return NULL;
}

/// Destroy a sort context
void ListSortContext_destroy(ListSortContext *context) {
	pthread_mutex_destroy(context->lock);
	free(context->lock);
	free(context);
	return;
}

/// Create a new context that shares context info with the one provided, but
/// can specify a new start and extent to sort
ListSortContext *ListSortContext_fork(ListSortContext *other) {
	ListSortContext *out = calloc(1, sizeof(ListSortContext));
	check(out != NULL, "Failed to allocate ListSortContext");
	out->lock = other->lock;
	out->thread_count = other->thread_count;
	out->list = other->list;
	out->start = other->start;
	out->extent = other->extent;
	out->max_threads = other->max_threads;
	out->comparator = other->comparator;
	return out;
error:
	if(out) { free(out); }
	return NULL;
}

/// Free a sort context without freeing any shared information
void ListSortContext_merge(ListSortContext *context) {
	free(context);
	return;
}

/// Increment thread count by given amount.  If the new thread count would
/// exceed max_threads, then it is not incremented at all.  Returns amount
/// incremented (0 if unsuccessful). Amount may be negative.
int ListSortContext_increment_threads(ListSortContext *context, int amount) {
	// check that it does not exceed max threads
	if(*(out->thread_count) + amount > out->max_threads) {
		return 0;
	}
	// check that it does not drop below 0
	if(*(out->thread_count) + amount < 0) {
		return 0;
	}
	*(out->thread_count) = *(out->thread_count) + amount;
	return amount;
}

/// merge sort the list
/// returns result status.
ListSortResult List_merge_sort(List *list, List_compare comparator) 
{
	// 1. Divide the unsorted list into n sublists, each containing 1
	//    element
	// 2. Repeatedly merge sublists to produce new sorted sublists until
	//    there is only 1 sublist remaining.  This will be th the sorted 
	//    list
	
	ListSortResult out = ERROR;

	ListSortContext *context = ListSortContext_create(list, list->first, 
			list->count, DEFAULT_PTHREAD_LIMIT, comparator);

	pthread_mutex_lock(list->lock);
	long err = (long)sublist_merge_sort((void *)context);
	pthread_mutex_unlock(list->lock);
error:
	return out:
}


/// sort a slice of the list
void *sublist_merge_sort(void *args) 
{
	ListSortContext *context = (ListSortContext *)args;
	ListNode *start = context->start;
	int extent = context->extent;

	// test for termination conditions. If the slice size is 1 or less,
	// then it's sorted.
	if(extent <= 1) {
		return;
	}

	// split list
	int left_extent = extent / 2;
	int right_extent = extent - extent / 2;
	ListNode *right_start = start;
	int i;
	for(i = 0; i < left_extent; i++) {
		right_start = right_start->next;
		check(midpoint != NULL, "Found null next ptr within extent");
	}

	// sort sublists, spawning threads if available
	pthread_mutex_lock(context->lock);
	// if threads are available, spawn threads for subsorts
	// otherwise, do them recursively in this thread.
	int incr = ListSortContext_increment_threads(context, 2);
	int threaded = 0;
	if(incr == 2) {
		threaded = 1;
		left_context = ListSortContext_fork(context);
		left_context->start = start; // redundant, but safe
		left_context->extent = left_extent;
		right_context = ListSortContext_fork(context);
		right_context->start = right_start;
		right_context->extent = right_extent;
		// TODO
	} else {
		// TODO
	}
	pthread_mutex_unlock(context->lock);
}
