#ifndef collect_List_algos_h
#define collect_List_algos_h

#include <collect/list.h>

#define SUCCESS_STATUS 0
#define FAIL_STATUS -1

typedef int (*List_compare)(void *lhs, void *rhs);

typedef struct ListSortContext {
	List *in;
	List *out;
	List_compare comparator;
} ListSortContext;

int List_bubble_sort(List *list, List_compare comparator);
List *List_merge_sort(List *list, List_compare comparator);
void *List_pt_merge_sort(void *args);

#endif
