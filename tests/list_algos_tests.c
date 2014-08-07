#include "minunit.h"
#include <collect/list_algos.h>
#include <collect/list.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

char *values[] = {"XXXX", "1234", "abcd", "xjvef", "NDSS"};
#define NUM_VALUES 5

#define LARGE_NUM_VALUES 1000000
#define SEED 42

List *create_words()
{
	int i = 0;
	List *words = List_create();

	for(i = 0; i < NUM_VALUES; i++) {
		List_push(words, values[i]);
	}

	return words;
}

List *create_large_numlist()
{
	int i = 0;
	List *nums = List_create();
	srand(SEED);
	int *n = malloc(LARGE_NUM_VALUES * sizeof(int));
	for(i = 0; i < LARGE_NUM_VALUES; i++) {
		*n = rand();
		List_push(nums, n);
		n++;
	}
	return nums;
}

int numcmp(int *l, int *r) {
	return *l - *r;
}

int is_sorted(List *words)
{
	LIST_FOREACH(words, first, next, cur) {
		if(cur->next && strcmp(cur->value, cur->next->value) > 0) {
			debug("%s %s", (char *)cur->value, 
					(char *)cur->next->value);
			return 0;
		}
	}

	return 1;
}


int is_numsorted(List *nums)
{
	LIST_FOREACH(nums, first, next, cur) {
		if(cur->next && numcmp(cur->value, cur->next->value) > 0) {
			debug("%d %d", *(int *)cur->value, 
					*(int *)cur->next->value);
			return 0;
		}
	}

	return 1;
}

char *test_bubble_sort()
{
	List *words = create_words();

	// should work on a list that needs sorting
	int rc = List_bubble_sort(words, (List_compare)strcmp);
	mu_assert(rc == 0, "Bubble sort failed.");
	mu_assert(is_sorted(words), "Words are not sorted after bubble sort.");

	// should work on an already sorted list
	rc = List_bubble_sort(words, (List_compare)strcmp);
	mu_assert(rc == 0, "Bubble sort of already sorted failed.");
	mu_assert(is_sorted(words), 
			"Words should be sorted if already bubble sorted.");

	List_destroy(words);

	// should work on an empty list
	words = List_create(words);
	rc = List_bubble_sort(words, (List_compare)strcmp);
	mu_assert(rc == 0, "Bubble sort failed on empty list.");
	mu_assert(is_sorted(words), "Words should be sorted if empty.");

	List_destroy(words);

	return NULL;
}

char *test_merge_sort()
{
	List *words = create_words();

	// should work on a list that needs sorting
	List *res = List_merge_sort(words, (List_compare)strcmp);
	mu_assert(is_sorted(res), "Words are not sorted after merge sort.");

	List *res2 = List_merge_sort(words, (List_compare)strcmp);
	mu_assert(is_sorted(res), "Should still be sorted after merge sort.");
	List_destroy(res2);
	List_destroy(res);

	List_destroy(words);
	return NULL;
}

char *test_large_merge_sort()
{
	List *nums = create_large_numlist();

	// should work on a list that needs sorting
	List *res = List_merge_sort(nums, (List_compare)numcmp);
	mu_assert(is_numsorted(res), "Words are not sorted after merge sort.");

	List *res2 = List_merge_sort(nums, (List_compare)numcmp);
	mu_assert(is_numsorted(res), "Should still be sorted after merge sort.");
	List_destroy(res2);
	List_destroy(res);

	List_destroy(nums);
	return NULL;
}


char *all_tests()
{
    mu_suite_start();

    mu_run_test(test_bubble_sort);
    mu_run_test(test_merge_sort);
    // we are going to take a break from this
    // mu_run_test(test_large_merge_sort);

    return NULL;
}

RUN_TESTS(all_tests);
