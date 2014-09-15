#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "LinkedList.h"


int
cmpFunc(void *my, void *nodeData) {
	assert(my != NULL && nodeData != NULL);

	return *((int *)my) - *((int *)nodeData);
}


int *
addInt(int dataArray[], size_t *p_index, int data) {
	assert(p_index != NULL);

	dataArray[*p_index] = data;
	return dataArray + (*p_index)++;
}


void
printListFromCursor(LinkedList *llist, LlistCursor *cursor) {
	int ret;

	do {
		void *p = llistCursor_getData(llist, cursor);
		assert(p != NULL);
		printf("Data: %d\n", *((int *)p));

		ret = llistCursor_getNext(llist, cursor);
		assert(ret > -2);
	} while (ret == 0);
	assert(ret == -1);
}


int
main(void) {
	int testData[100] = { 0 };
	size_t idx = 0;

	LinkedList *llist = llist_new(NULL, cmpFunc);
	LlistCursor *cursor = llistCursor_new();

	assert(0 == llist_insertTail(llist, addInt(testData, &idx, 9)));
	assert(0 == llist_insertTail(llist, addInt(testData, &idx, 1)));
	assert(0 == llist_insertTail(llist, addInt(testData, &idx, 6)));
	assert(0 == llist_insertTail(llist, addInt(testData, &idx, 3)));
	assert(0 == llist_insertTail(llist, addInt(testData, &idx, 8)));
	assert(0 == llist_insertTail(llist, addInt(testData, &idx, 5)));
	assert(0 == llist_insertTail(llist, addInt(testData, &idx, 8)));

	assert(llistCursor_getHead(llist, cursor) == 0);
	assert(llistCursor_isHead(llist, cursor) == 0);
	printListFromCursor(llist, cursor);
	assert(llistCursor_isTail(llist, cursor) == 0);

	assert(llistCursor_getNext(llist, cursor) == -1);
	assert(llistCursor_getPrev(llist, cursor) == 0);

	assert(llistCursor_isTail(llist, cursor) == 1);

	assert(llistCursor_getHead(llist, cursor) == 0);


	{
		int nb = 6;

		assert(llistCursor_findNext(llist, cursor, &nb) == 0);
		assert(llistCursor_findNext(llist, cursor, &nb) == -1);

		nb = 8;
		assert(llistCursor_findNext(llist, cursor, &nb) == 0);
		assert(llistCursor_findNext(llist, cursor, &nb) == 0);
		assert(llistCursor_findNext(llist, cursor, &nb) == -4);

		assert(llistCursor_isTail(llist, cursor) == 0);

		nb = 3;
		assert(llist_countMatch(llist, &nb) == 1);
		printf("1 occurence of '3' in list\n");
		nb = 8;
		assert(llist_countMatch(llist, &nb) == 2);
		printf("2 occurences of '8' in list\n");
	}

	printf("It's bubblesort time!\n");
	llist_bubbleSort(llist);
	assert(llistCursor_getHead(llist, cursor) == 0);
	printListFromCursor(llist, cursor);

	assert(llistCursor_destroy(&cursor) == 0);
	assert(llist_destroy(&llist) == 0);


	return 0;
}
