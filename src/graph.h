#ifndef GRAPH_H
#define GRAPH_H

#include "types.h"
#include "cgraph.h"

typedef f64 (*func)(f64);

typedef struct {
	SessionInfo *info;

	bool *data;
	usize data_size;

	func function;

	ui32 color;
	bool visible;
} Graph;

Graph graph_create(SessionInfo *info, func function, ui32 color);

void graph_plot(Graph *graph);

void graph_destroy(Graph *graph);

#endif
