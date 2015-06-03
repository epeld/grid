#pragma once

struct GRID_node_struct {
	void*	data;

	struct GRID_node_struct* next;
};

typedef struct GRID_node_struct GRID_QueueNode;


typedef struct {
	GRID_QueueNode*	first;
} GRID_Queue;

void	GRID_queuePut( GRID_Queue* q, void* data );
void*	GRID_queueGet( GRID_Queue* q );

int	GRID_queueIsEmpty( GRID_Queue* q );

void*	GRID_queuePeek( GRID_Queue* q );

void	GRID_queueClean( GRID_Queue* q, void (*fkn)(void*, void*) );
