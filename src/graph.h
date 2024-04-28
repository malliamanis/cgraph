#ifndef GRAPH_H
#define GRAPH_H

#include <stddef.h>
#include <stdbool.h>

#include "cgraph.h"

typedef double (*func)(double);

typedef struct {
	const SessionInfo *info;

	bool *data;
	size_t data_size;

	func function;

	uint32_t color;
	bool visible;
} Graph;

Graph graph_create(const SessionInfo *info, func function, uint32_t color);

void graph_plot(Graph *graph);

void graph_destroy(Graph *graph);

#endif
