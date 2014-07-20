#ifndef collect_List_algos_h
#define collect_List_algos_h

#include <collect/list.h>

typedef int (*List_compare)(void *lhs, void *rhs);

int List_bubble_sort(List *list, List_compare comparator);
List *List_merge_sort(List *list, List_compare comparator);

#endif
