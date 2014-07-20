#include <collect/list_algos.h>
#include <dbg.h>

typedef int (*List_compare)(void *lhs, void *rhs);

int List_bubble_sort(List *list, List_compare comparator) 
{
	// 1. for each item in the list:
	//	a) if the item is greater than the next item, swap them
	// 2. repeat until a pass through the list where no swaps are needed
	int done = 0;
	// keep going until we get a pass with no switches
	while(!done) {
		done = 1;
		// Iterate through each item in the list
		LIST_FOREACH(list, first, next, cur) {
			// if right hand side is less than left hand side
			if(cur->next && comparator(cur->value, 
						cur->next->value) > 0) {
				done = 0;
				void *tmp = cur->next->value;
				cur->next->value = cur->value;
				cur->value = tmp;
			}
		}
	}
	return 0;
}


List *List_merge_sort(List *list, List_compare comparator) 
{
	// From Wikipedia:
	// 1. Divide the unsorted list into n sublists, each containing 1
	//    element
	// 2. Repeatedly merge sublists to produce new sorted sublists until
	//    there is only 1 sublist remaining.  This will be th the sorted 
	//    list

	List *out = List_create();
	List *left = NULL;
	List *right = NULL;
	List *sorted_left = NULL;
	List *sorted_right = NULL;

	// Check for single-node or empty lists, which are already sorted
	if(list->count < 2) {
		if(list->count == 1) {
			List_push(out, list->first->value);
		}
		return out;
	}

	// Divide list into two new lists
	left = List_create();
	right = List_create();
	ListNode *cur = list->first;
	int i;
	for(i = 0; i < list->count / 2; i++) {
		check(cur != NULL, "Got a null value for a node in the list "
			"with index less than list->count");
		List_push(left, cur->value);
		cur = cur->next;
	}
	for(; i < list->count; i++) {
		check(cur != NULL, "Got a null value for a node in the list "
				"with index less than list->count");
		List_push(right, cur->value);
		cur = cur->next;
	}

	// debug("list->count:\t%d", list->count);
	// debug("left->count:\t%d", left->count);
	// debug("right->count:\t%d", right->count);
	check(list->count == left->count + right->count, "left and right list "
			"split sizes do not add up to input size.");

	// merge sort each
	sorted_left = List_merge_sort(left, comparator);
	sorted_right = List_merge_sort(right, comparator);

	// debug("sorted_left->count:\t%d", sorted_left->count);
	// debug("sorted_right->count:\t%d", sorted_right->count);
	check(left->count == sorted_left->count, "Internal sorted list had a "
			"different count than its input");
	check(right->count == sorted_right->count, "Internal sorted list had a "
			"different count than its input");

	// merge sorted lists
	void *lval = NULL;
	void *rval = NULL;
	int lcount = sorted_left->count;
	int rcount = sorted_right->count;
	while(sorted_left->count > 0 || sorted_right->count > 0 
			|| lval != NULL || rval != NULL) {
		// load left and right values from START
		if(lval == NULL && sorted_left->count > 0) {
			lval = List_shift(sorted_left);
		}
		if(rval == NULL && sorted_right->count > 0) {
			rval = List_shift(sorted_right);
		}
		// append lesser of the values to out
		if(lval == NULL) {
			List_push(out, rval);
			rval = NULL;
			continue;
		}
		if(rval == NULL) {
			List_push(out, lval);
			lval = NULL;
			continue;
		}
		// if comparator < 0 then lval < rval
		if(comparator(lval, rval) < 0) {
			List_push(out, lval);
			lval = NULL;
		} else {
			List_push(out, rval);
			rval = NULL;
		}
	}

	// debug("out->count:\t%d", out->count);

	check(out->count == lcount + rcount, "merged list count was not equal "
			"to size of right + left.");
	check(out->count == list->count, "merged list count was not equal to "
			"size of input.");

error:
	if(left != NULL) { List_destroy(left); }
	if(right != NULL) { List_destroy(right); }
	if(sorted_left != NULL) { List_destroy(sorted_left); }
	if(sorted_right != NULL) { List_destroy(sorted_right); }
	return out;
}
