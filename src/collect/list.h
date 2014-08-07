#ifndef collect_List_h
#define collect_List_h

#include <stdlib.h>
#include <pthread.h>

struct ListNode;

typedef struct ListNode {
	struct ListNode *next;
	struct ListNode *prev;
	void *value;
} ListNode;

typedef struct List {
	pthread_mutex_t *lock;
	int count;
	ListNode *first;
	ListNode *last;
} List;

typedef int (*List_compare)(void *lhs, void *rhs);

List *List_create();
void List_destroy(List *list);
void List_clear(List *list);
void List_clear_destroy(List *list);

#define List_count(A) ((A)->count)
#define List_first(A) ((A)->first != NULL ? (A)->first->value : NULL)
#define List_last(A) ((A)->last != NULL ? (A)->last->value : NULL)

void List_push(List *list, void *value);
void *List_pop(List *list);

void List_unshift(List *list, void *value);
void *List_shift(List *list);

void *List_remove(List *list, ListNode *node);

void *List_merge_sort(List *list, List_compare comparator);
/*
 *void *List_merge_sort_threaded(
 *                List *list, 
 *                List_compare comparator, 
 *                int thread_limit
 *);
 */
#define LIST_FOREACH(L, S, M, V) ListNode *_node = NULL;\
	ListNode *V = NULL;\
	for(V = _node = L->S; _node != NULL; V = _node = _node->M)

#endif
