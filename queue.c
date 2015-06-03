#include "queue.h"
#include "stdlib.h"

static void
nodePut( GRID_QueueNode* n, void* data ) {
	if( n->next ) {
		nodePut( n->next, data );
		return;
	}
	GRID_QueueNode *n2 = (GRID_QueueNode*)malloc( sizeof( GRID_QueueNode ) );
	n->next = n2;
	n2->next = NULL;
	n2->data = data;
}

void
GRID_queuePut( GRID_Queue* q, void* data ) {
	if( ! q->first ) {
		GRID_QueueNode *n = malloc( sizeof(GRID_QueueNode ) );
		n->next = NULL;
		n->data = data;
		q->first = n;
		return;
	}
	nodePut( q->first, data );
}

void*
GRID_queueGet( GRID_Queue* q ) {
	GRID_QueueNode* n = q->first;
	q->first = n->next;

	void* data = n->data;
	free( n );

	return data;
}

void*
GRID_queuePeek( GRID_Queue* q ) {

	return q->first->data;
}

int	GRID_queueIsEmpty( GRID_Queue* q ){ return q->first ? 0 : 1; }
