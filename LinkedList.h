/*
 * Author: Olivier Diotte
 * Date of birth: 2014/09/11
 */

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h> /* size_t */

typedef enum e_LlistDirection {
	LLIST_BEFORE = -1,
	LLIST_HERE = 0,
	LLIST_AFTER = 1,
	LLIST_NB_DIR
} LlistDirection;


typedef int (*nodeDestroyFunc)(void *);
typedef int (*nodeCmpFunc)(void *, void *);


/* Forward declare and typedef internal structs (since callers shouldn't know the internals) */
typedef struct Node *LlistCursor;
typedef struct s_LinkedList LinkedList;



/* === ctor/dtor === */

LinkedList *
llist_new(nodeDestroyFunc f_destroyNode, nodeCmpFunc f_cmpNode);


int
llist_destroy(LinkedList **p_llist);

/* === END ctor/dtor === */



/* === cursor functions === */

LlistCursor *
llistCursor_new(void);

LlistCursor *
llistCursor_copy(LlistCursor *);

int
llistCursor_destroy(LlistCursor **p_cursor);

int
llistCursor_setData(LinkedList *llist, LlistCursor *cursor, void *newData);

void *
llistCursor_getData(LinkedList *llist, LlistCursor *cursor);

int
llistCursor_getNext(LinkedList *llist, LlistCursor *cursor);

int
llistCursor_getPrev(LinkedList *llist, LlistCursor *cursor);

int
llistCursor_findNext(LinkedList *llist, LlistCursor *cursor, void *data);

int
llistCursor_findPrev(LinkedList *llist, LlistCursor *cursor, void *data);

int
llistCursor_find(LinkedList *llist, LlistCursor *cursor, void *data, LlistDirection searchDir);

int
llistCursor_getHead(LinkedList *llist, LlistCursor *cursor);

int
llistCursor_getTail(LinkedList *llist, LlistCursor *cursor);

int
llistCursor_isTail(LinkedList *llist, LlistCursor *cursor);

int
llistCursor_isHead(LinkedList *llist, LlistCursor *cursor);

int
llistCursor_insertData(LinkedList *llist, LlistCursor *cursor, void *data, LlistDirection dir);

/* === END cursor functions === */



/* === Query functions === */

size_t
llist_countMatch(LinkedList *llist, void *data);

void *
llist_getHeadData(LinkedList *llist);

void *
llist_getTailData(LinkedList *llist);

/* === END Query functions === */


/* === Mutator functions === */
int
llist_bubbleSort(LinkedList *llist);
/* === END Mutator functions === */


/* === Insert functions === */
int
llist_insertHead(LinkedList *llist, void *data);

int
llist_insertTail(LinkedList *llist, void *data);

/* === END Insert functions === */


/* === Delete functions === */
void *
llist_popNode(LinkedList *llist, LlistCursor *node);

int
llist_removeNode(LinkedList *llist, LlistCursor *node);

void *
llist_popHead(LinkedList *llist);

void *
llist_popTail(LinkedList *llist);

/* === END Delete functions === */

#endif /* Guard */
