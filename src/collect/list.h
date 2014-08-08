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

#ifndef collect_List_h
#define collect_List_h

#include <stdlib.h>
#include <pthread.h>


struct ListNode;

/// A Node within a Linked List.
typedef struct ListNode {
	struct ListNode *next;
	struct ListNode *prev;
	void *value;
} ListNode;

/// A Doubly Linked List.
typedef struct List {
	pthread_mutex_t *lock;
	int count;
	ListNode *first;
	ListNode *last;
} List;

typedef int (*List_compare)(void *lhs, void *rhs);


/// Allocate a new list from the heap.
List *List_create();

/// Free a list, as well as any nodes belonging to it.
/**
 * List_destroy frees list resources, but does not free the values of its
 * nodes. Refer to List_clear and List_clear_destroy for freeing node values.
 */
void List_destroy(List *list);

/// Free all values contained by the list.
/**
 * List_clear frees the values contained by a list, but does not free the list
 * or its nodes. Refer to List_destroy and list_clear_destroy for freeing list 
 * structures.
 */
void List_clear(List *list);

/// Free a list, its nodes, and any values it contains.
/**
 * List_clear_destroy frees a list structure, as well as the values it
 * contains.  It is equivalent to calling List_clear followed by List_destroy.
 */
void List_clear_destroy(List *list);


#define List_count(A) ((A)->count)
#define List_first(A) ((A)->first != NULL ? (A)->first->value : NULL)
#define List_last(A) ((A)->last != NULL ? (A)->last->value : NULL)


/// retrieve the value stored at an index
void *List_get(List *list, int index);


/// push a new value onto the end of the list.
void List_push(List *list, void *value);

/// remove and return the end of the list.
void *List_pop(List *list);


/// push a new value onto the beginning of the list.
void List_unshift(List *list, void *value);

/// remove and return the beginning of the list.
void *List_shift(List *list);


/// remove and return a specified node from the list.
void *List_remove(List *list, ListNode *node);


/// merge sort the list
void *List_merge_sort(List *list, List_compare comparator);


/// convenience for loop iterating across a list.
/**
 * LIST_FOREACH creates a forloop that iterates across List L.
 * @param L the list to iterate on.
 * @param S the side of the list to start on. (first|last)
 * @param M the direction in which to move. (next|prev)
 * @param V the name to use for the pointer to the current node.
 */
#define LIST_FOREACH(L, S, M, V) ListNode *_node = NULL;\
	ListNode *V = NULL;\
	for(V = _node = L->S; _node != NULL; V = _node = _node->M)

#endif
