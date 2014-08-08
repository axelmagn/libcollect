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


typedef struct ListSortContext {
	List *in;
	List *out;
	List_compare comparator;
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


/// retrieve the value stored at an index
void *List_get(List *list, int index) {
	return List_get_node(list, index)->value;
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
	return node;
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


/// retrieve the value stored at an index
void *List_get(List *list, int index) {
}


/// merge sort the list
void List_merge_sort(List *list, List_compare comparator) 
{
	// 1. Divide the unsorted list into n sublists, each containing 1
	//    element
	// 2. Repeatedly merge sublists to produce new sorted sublists until
	//    there is only 1 sublist remaining.  This will be th the sorted 
	//    list
}


/// sort a slice of the list
void sublist_merge_sort(ListNode *start, int extent) {
	// test for termination conditions. If the slice size is 1 or less,
	// then it's sorted.
	if(extent <= 1) {
		return;
	}

	// split list
}
