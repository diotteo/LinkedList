/*
 * Author: Olivier Diotte
 * Date of birth: 2014/09/11
 */

#include <stdlib.h>
#include <assert.h>

#include "LinkedList.h"


#ifndef DEBUG
	#define DEBUG 0
#endif


struct Node {
	struct Node *prev;
	struct Node *next;
	void *data;
};


struct s_LinkedList {
	struct Node *head;
	struct Node *tail;
	nodeDestroyFunc f_destroyNode;
	nodeCmpFunc f_cmpNode;
};



/* === Internal functions === */
static void
assertList(LinkedList *llist) {
	if (DEBUG) {
		assert(llist != NULL);

		if (llist->head == NULL || llist->tail == NULL) {
			assert(llist->head == NULL && llist->tail == NULL);
		} else {
			assert(llist->head != NULL && llist->tail != NULL);
		}
	}
}


static int
isUserPointerValid(struct Node **cursor) {
	return (cursor != NULL && *cursor != NULL);
}


static struct Node *
new_node(void *data) {
	struct Node *node = malloc(sizeof (*node));

	if (node != NULL) {
		node->prev = node->next = NULL;
		node->data = data;
	}

	return node;
}


static int
destroyNode(nodeDestroyFunc f_destroyNode, struct Node **p_node) {
	int destroyCode;
	struct Node *node;

	/* Not actually a user pointer, just a double pointer to Node this time */
	if (!isUserPointerValid(p_node)) {
		return -256;
	}

	node = *p_node;

	if (f_destroyNode != NULL) {
		destroyCode = f_destroyNode(node->data);
	}
	free(node), *p_node = node = NULL;

	return destroyCode;
}


static int
isTail(LinkedList *llist, struct Node *node) {
	assertList(llist);
	/* node can be NULL only if the list is empty */
	assert(node != NULL || llist->tail == NULL);

	if (llist->tail == node || node->next == NULL) {
		assert(llist->tail == node && (node == NULL || node->next == NULL));
		return 1;

	} else {
		assert(llist->tail != node && node->next != NULL);
		return 0;
	}
}


static int
isHead(LinkedList *llist, struct Node *node) {
	assertList(llist);
	/* node can be NULL only if the list is empty */
	assert(node != NULL || llist->head == NULL);

	if (llist->head == node || node->prev == NULL) {
		assert(llist->head == node && (node == NULL || node->prev == NULL));
		return 1;
	} else {
		assert(llist->head != node && node->prev != NULL);
		return 0;
	}
}


static int
insertNode(LinkedList *llist, struct Node **p_insertPos, struct Node *newNode, LlistDirection dir) {
	int isEdge = 0;

	assertList(llist);
	/* p_insertPos CAN'T be NULL (although insertPos can be NULL if the list is empty) */
	assert(p_insertPos != NULL && newNode != NULL);
	assert(newNode->next == NULL && newNode->prev == NULL);

	switch (dir) {
	case LLIST_BEFORE:
		newNode->next = *p_insertPos;

		/* Inserting before the head node (*p_insertPos is NULL if list is empty) */
		if (isHead(llist, *p_insertPos)) {
			isEdge = 1;
			newNode->prev = NULL;

		/* Inserting before some other node */
		} else {
			/* We should have more than one element */
			assert(llist->head != llist->tail);

			newNode->prev = (*p_insertPos)->prev;

			/* The Node before the insertPos was still pointing to insertPos */
			(*p_insertPos)->prev->next = newNode;
		}

		if (*p_insertPos != NULL) {
			(*p_insertPos)->prev = newNode;
		}

		/* We need to touch the head only after everything is done
		 * otherwise we will set the head (setting *p_insertPos at the same time in some cases) */
		if (isEdge) {
			llist->head = newNode;

			if (llist->tail == NULL) {
				llist->tail = newNode;
			}
		}
		break;
	case LLIST_AFTER:
		newNode->prev = *p_insertPos;

		/* Inserting after the tail node (*p_insertPos is NULL if list is empty) */
		if (isTail(llist, *p_insertPos)) {
			isEdge = 1;
			newNode->next = NULL;

		/* Inserting after some other node */
		} else {
			/* We should have more than one element */
			assert(llist->head != llist->tail);

			newNode->next = (*p_insertPos)->next;

			/* The Node after the insertPos was still pointing to insertPos */
			(*p_insertPos)->next->prev = newNode;
		}

		if (*p_insertPos != NULL) {
			(*p_insertPos)->next = newNode;
		}

		/* See comment about head above */
		if (isEdge) {
			llist->tail = newNode;

			if (llist->head == NULL) {
				llist->head = newNode;
			}
		}
		break;
	default:
		return -1;
		break;
	}

	return 0;
}


static int
insertHead(LinkedList *llist, struct Node *node) {
	return insertNode(llist, &(llist->head), node, LLIST_BEFORE);
}


static int
insertTail(LinkedList *llist, struct Node *node) {
	return insertNode(llist, &(llist->tail), node, LLIST_AFTER);
}


static struct Node *
popNode(LinkedList *llist, struct Node *node) {
	struct Node *prev, *next;

	assertList(llist);
	assert(node != NULL);

	prev = node->prev;
	next = node->next;

	if (isHead(llist, node)) {
		llist->head = next;
		if (next != NULL) {
			next->prev = NULL;
		}

	} else if (isTail(llist, node)) {
		llist->tail = prev;
		if (prev != NULL) {
			prev->next = NULL;
		}

	} else {
		next->prev = prev;
		prev->next = next;
	}

	return node;
}


static struct Node *
popHeadNode(LinkedList *llist) {
	return popNode(llist, llist->head);
}


static struct Node *
popTailNode(LinkedList *llist) {
	return popNode(llist, llist->tail);
}


static void
swapNodes(LinkedList *llist, struct Node *node1, struct Node *node2) {
	int b_neighbours;
	struct Node
			*prev1,
			*next1,
			*prev2,
			*next2;

	assert(node1 != NULL && node2 != NULL);

	/* If both nodes are next to each other, make sure
	 * node1 refers to the first one */
	if (node1->prev == node2) {
		struct Node *tmp = node1;
		node1 = node2;
		node2 = tmp;
	}

	/* Sync test */
	assert( ! ((node1->next == node2) ^ (node2->prev == node1)));


	prev1 = node1->prev;
	next1 = node1->next;
	next2 = node2->next;
	prev2 = node2->prev;

	b_neighbours = (node1->next == node2);

	if (prev1 != NULL) {
		prev1->next = node2;
	}
	if (next2 != NULL) {
		next2->prev = node1;
	}

	if (b_neighbours) {
		node1->prev = node2;
		node2->next = node1;

	} else {
		if (next1 != NULL) {
			next1->prev = node2;
		}
		if (prev2 != NULL) {
			prev2->next = node1;
		}

		node1->prev = prev2;
		node2->next = next1;
	}

	node2->prev = prev1;
	node1->next = next2;


	if (llist->head == node1) {
		llist->head = node2;
	} else if (llist->head == node2) {
		llist->head = node1;
	}

	if (llist->tail == node1) {
		llist->tail = node2;
	} else if (llist->tail == node2) {
		llist->tail = node1;
	}
}


/* === END Internal functions === */



LinkedList *
llist_new(nodeDestroyFunc f_destroyNode, nodeCmpFunc f_cmpNode) {
	LinkedList *llist = malloc(sizeof (*llist));

	assert(llist != NULL);

	if (llist != NULL) {
		llist->head = NULL;
		llist->tail = NULL;

		llist->f_destroyNode = f_destroyNode;
		llist->f_cmpNode = f_cmpNode;
	}

	return llist;
}


int
llist_destroy(LinkedList **p_llist) {
	struct Node *node;
	int error = 0;
	LinkedList *llist;

	if (p_llist == NULL) {
		return 0;
	}

	assertList(*p_llist);

	llist = *p_llist;
	while (llist->head != NULL) {
		int destroyCode;

		#if 0
		assert(llist->nbNodes > 0);
		#endif
		node = popHeadNode(llist);
		destroyCode = destroyNode(llist->f_destroyNode, &node);
		if (destroyCode != 0) {
			error = destroyCode;
		}
		#if 0
		llist->nbNodes--;
		#endif
	}

	free(*p_llist), *p_llist = llist = NULL;
	return error;
}


int
llistCursor_isTail(LinkedList *llist, struct Node **cursor) {
	struct Node *node;

	if (!isUserPointerValid(cursor)) {
		return -1;
	}
	node = *cursor;
	return !isTail(llist, node);
}


int
llistCursor_isHead(LinkedList *llist, struct Node **cursor) {
	struct Node *node;

	if (!isUserPointerValid(cursor)) {
		return -1;
	}
	node = *cursor;

	return !isHead(llist, node);
}


/* Returns 0 on success, negative number on failure */
int
llist_insertHead(LinkedList *llist, void *data) {

	return insertHead(llist, new_node(data));
}


/* Returns 0 on success, negative number on failure */
int
llist_insertTail(LinkedList *llist, void *data) {
	return insertTail(llist, new_node(data));
}


/* Returns 0 on success, negative number on failure */
int
llistCursor_insertData(LinkedList *llist, struct Node **cursor, void *data, LlistDirection dir) {
	struct Node *newNode = new_node(data);

	if (!isUserPointerValid(cursor)) {
		return -2;
	}

	return insertNode(llist, cursor, newNode, dir);
}


/* Caller is responsible of freeing the data returned */
void *
llist_popNode(LinkedList *llist, struct Node **cursor) {
	void *data;
	struct Node *node;

	if (!isUserPointerValid(cursor)) {
		return NULL;
	}
	node = *cursor;

	assert(node == popNode(llist, node));

	data = node->data;
	destroyNode(NULL, cursor);
	return data;
}


int
llist_removeNode(LinkedList *llist, struct Node **cursor) {
	struct Node *node;

	assertList(llist);
	if (!isUserPointerValid(cursor)) {
		return -1;
	}

	node = popNode(llist, *cursor);
	assert(node == *cursor);

	return destroyNode(llist->f_destroyNode, cursor);
}


/* Returns NULL in case of error
 * Caller is responsible of freeing data
 */
void *
llist_popHead(LinkedList *llist) {
	assertList(llist);

	if (llist == NULL) {
		return NULL;
	}
	return popHeadNode(llist);
}


/* Returns NULL in case of error
 * Caller is responsible of freeing data
 */
void *
llist_popTail(LinkedList *llist) {
	assertList(llist);

	if (llist == NULL) {
		return NULL;
	}
	return popTailNode(llist);
}


/*
 * llist_countMatch
 *
 * llist: LinkedList to search into
 * data: data to match such that llist->f_cmpNode(nodeData, data) == 0
 */
size_t
llist_countMatch(LinkedList *llist, void *data) {
	struct Node *node;
	size_t count = 0;

	for (node = llist->head; node != NULL; node = node->next) {
		if (0 == llist->f_cmpNode(data, node->data)) {
			count++;
		}
	}

	return count;
}


int
llistCursor_findNext(LinkedList *llist, struct Node **cursor, void *data) {
	if (llistCursor_getNext(llist, cursor) != 0) {
		return -4;
	}
	return llistCursor_find(llist, cursor, data, LLIST_AFTER);
}


int
llistCursor_findPrev(LinkedList *llist, struct Node **cursor, void *data) {
	if (llistCursor_getPrev(llist, cursor) != 0) {
		return -4;
	}
	return llistCursor_find(llist, cursor, data, LLIST_BEFORE);
}


/*
 * llistCursor_find
 *
 * llist: LinkedList to search into
 * searchBase: the Node to start the search at
 * data: data to match such that llist->f_cmpNode(data, nodeData) == 0
 * searchDir: the direction to search in
 */
int
llistCursor_find(LinkedList *llist, struct Node **cursor, void *data, LlistDirection searchDir) {
	struct Node *node;

	assertList(llist);
	if (!isUserPointerValid(cursor)) {
		return -2;
	}

	for (node = *cursor; node != NULL; ) {
		if (0 == llist->f_cmpNode(data, node->data)) {
			*cursor = node;
			return 0;
		}

		switch (searchDir) {
		case LLIST_BEFORE:

			node = node->prev;
			break;
		case LLIST_AFTER:

			node = node->next;
			break;
		default:
			return -3;
		}
	}

	assert(node == NULL);
	return -1;
}







int
llist_bubbleSort(LinkedList *llist) {
	struct Node *node, *prev;

	assertList(llist);

	/* Already sorted */
	if (llist->head == NULL || llist->head->next == NULL) {
		return 0;
	}

	for (prev = llist->head, node = llist->head->next; node != NULL; prev = node, node = node->next) {
		struct Node *lastSorted = NULL;

		/* while node is not at the head of the list and prev is "higher" than node */
		while (prev != NULL && llist->f_cmpNode(prev->data, node->data) > 0) {
			swapNodes(llist, prev, node);
			assert(prev->prev == node && node->next == prev);

			/* prev is now after node */
			if (lastSorted == NULL) {
				lastSorted = prev;
			}
			/* update prev to be the new previous node (prev->prev before the swap) */
			prev = node->prev;
		}

		if (lastSorted != NULL) {
			node = lastSorted;
			/* no need to update prev (since it will just get written to), but let's do it anyway */
			prev = node->prev;
		}
	}

	return 0;
}


void *
llist_getHeadData(LinkedList *llist) {
	assertList(llist);

	return llist->head->data;
}


void *
llist_getTailData(LinkedList *llist) {
	assertList(llist);

	return llist->tail->data;
}


/* === Cursor functions === */
/* We can't return Nodes to the user, that would be ABI dependant.
 * We can't return pointers to Nodes, because we can't set the user pointer to NULL when we free Nodes
 * BUT with a pointer to a pointer to Node, the user can user their "LlistCursor pointer" anyway they want
 * since that pointer points to a single pointer which CAN be invalidated when the user asks for Node removal
 *
 * userCursorPointer1 \
 *
 * userCursorPointer2 -- Node pointer -> Node
 *
 * userCursorPointer3 /
 *
 *
 * After removing Node:
 *
 * userCursorPointer1 \
 *
 * userCursorPointer2 -- Node pointer -> NULL
 *
 * userCursorPointer3 /
 *
 *
 * By asking for a pointer to cursor pointer (pointer to pointer to pointer to Node)
 * we can also invalidate the reference passed to us for the user but we still
 * can't invalidate the copies: it is the user's responsibility to ensure their
 * side is bugfree
 *
 * userCursorPointer1 -> NULL
 *
 * userCursorPointer2 -- freed cursor
 *
 * userCursorPointer3 /
 *
 *
 *
 * prevNode next pointer \
 *
 * nextNode prev pointer - Node
 *
 * userNodePointer ------/
 *
 *
 * After removing Node:
 *
 * prevNode next pointer -> NULL
 *
 * nextNode prev pointer -> NULL
 *
 * userNodePointer -------> freed space
 *
 */
struct Node **
llistCursor_new(void) {
	struct Node **cursor = malloc(sizeof (*cursor));

	if (cursor != NULL) {
		*cursor = NULL;
	}
	return cursor;
}


int
llistCursor_destroy(struct Node ***p_cursor) {
	if (p_cursor != NULL) {
		if (*p_cursor == NULL) {
			return 0;
		}
		free(*p_cursor), *p_cursor = NULL;
		return 0;
	}
	return -1;
}


/* llist is not needed now, but for consistency and forward-compatibility,
 * it's probably better to require it anyway */
int
llistCursor_setData(LinkedList *llist, struct Node **cursor, void *newData) {

	assertList(llist);

	if (!isUserPointerValid(cursor)) {
		return -1;
	}

	(*cursor)->data = newData;
	return 0;
}


/* llist is not needed now, but for consistency and forward-compatibility,
 * it's probably better to require it anyway */
void *
llistCursor_getData(LinkedList *llist, struct Node **cursor) {
	struct Node *node;

	assertList(llist);

	if (!isUserPointerValid(cursor)) {
		return NULL;
	}
	node = *cursor;

	return node->data;
}


int
llistCursor_getHead(LinkedList *llist, struct Node **cursor) {
	assertList(llist);

	if (cursor == NULL) {
		return -1;
	}

	*cursor = llist->head;
	return 0;
}


int
llistCursor_getTail(LinkedList *llist, struct Node **cursor) {
	assertList(llist);

	if (cursor == NULL) {
		return -1;
	}

	*cursor = llist->tail;
	return 0;
}


int
llistCursor_getPrev(LinkedList *llist, struct Node **cursor) {

	if (!isUserPointerValid(cursor)) {
		return -2;
	}

	if ((*cursor)->prev == NULL) {
		return -1;
	}
	*cursor = (*cursor)->prev;
	return 0;
}


int
llistCursor_getNext(LinkedList *llist, struct Node **cursor) {

	if (!isUserPointerValid(cursor)) {
		return -2;
	}

	if ((*cursor)->next == NULL) {
		return -1;
	}

	*cursor = (*cursor)->next;
	return 0;
}
/* === END Cursor functions === */
