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


List *List_merge_sort(List *list, List_compare comparator) {
	ListSortContext context;
	context.in = list;
	context.out = NULL;
	context.comparator = comparator;

	pthread_t sort_pt;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int rc;
	rc = pthread_create(&sort_pt, &attr, List_pt_merge_sort, 
			(void *)&context);
	check(rc == 0, "Return code from pthread_create() on merge sort is %d", 
			rc);

	void *sort_status;
	rc = pthread_join(sort_pt, &sort_status);
	check(rc == 0, "Return code from pthread_join() on merge sort is %d", 
			rc);
	check((long)sort_status == SUCCESS_STATUS, "Exit status for merge sort "
			"is %ld", (long)sort_status);
error:
	pthread_attr_destroy(&attr);
	return context.out;
}


void *List_pt_merge_sort(void *args) 
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
	long status = FAIL_STATUS;
	int  list_locked = 0;

	// cast context
	ListSortContext *context = (ListSortContext *)args;
	List *list = context->in;
	List_compare comparator = context->comparator;
	check(list != NULL, "Input list was NULL");
	
	// This is a thread call, so we lock the shared data
	// we also need to track locally whether the thread is locked
	pthread_mutex_lock(list->lock);
	list_locked = 1;

	// store this variable for later use
	int list_count = list->count;

	// Check for single-node or empty lists, which are already sorted
	if(list->count < 2) {
		if(list->count == 1) {
			List_push(out, list->first->value);
		}
		context->out = out;
		pthread_mutex_unlock(list->lock);
		pthread_exit(SUCCESS_STATUS);
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

	// unlock the list after reading.  we don't need it
	pthread_mutex_unlock(list->lock);
	list_locked = 0;

	// debug("list->count:\t%d", list->count);
	// debug("left->count:\t%d", left->count);
	// debug("right->count:\t%d", right->count);
	check(list_count == left->count + right->count, "left and right list "
			"split sizes do not add up to input size.");

	// merge sort each
	pthread_t left_sort_pt;
	pthread_t right_sort_pt;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	ListSortContext left_sort_ctx;
	left_sort_ctx.in = left;
	left_sort_ctx.out = NULL;
	left_sort_ctx.comparator = comparator;

	ListSortContext right_sort_ctx;
	right_sort_ctx.in = right;
	right_sort_ctx.out = NULL;
	right_sort_ctx.comparator = comparator;

	int rc;
	rc = pthread_create(&left_sort_pt, &attr, List_pt_merge_sort, 
			(void *)&left_sort_ctx);
	check(rc == 0, "Return code from pthread_create() on left sort is %d", 
			rc);
	rc = pthread_create(&right_sort_pt, &attr, List_pt_merge_sort,
			(void *) &right_sort_ctx);
	check(rc == 0, "Return code from pthread_create() on right sort is %d", 
			rc);

	pthread_attr_destroy(&attr);

	void *sort_status;
	rc = pthread_join(left_sort_pt, &sort_status);
	check(rc == 0, "Return code from pthread_join() on left sort is %d", 
			rc);
	check(sort_status == SUCCESS_STATUS, "Exit status for left sort is %ld", 
			(long)sort_status);

	rc = pthread_join(right_sort_pt, &sort_status);
	check(rc == 0, "Return code from pthread_join() on right sort is %d", 
			rc);
	check(sort_status == SUCCESS_STATUS, "Exit status for right sort is %ld", 
			(long)sort_status);


	sorted_left = left_sort_ctx.out;
	sorted_right = right_sort_ctx.out;
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

	status = SUCCESS_STATUS;

error:
	if(left != NULL) { List_destroy(left); }
	if(right != NULL) { List_destroy(right); }
	if(sorted_left != NULL) { List_destroy(sorted_left); }
	if(sorted_right != NULL) { List_destroy(sorted_right); }
	if(list_locked) { pthread_mutex_unlock(list->lock); }
	context->out = out;
	pthread_exit((void *)status);
}
