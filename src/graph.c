#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"

Graph graph_create(const SessionInfo *info, func function, uint32_t color)
{
	Graph g = {
		.info = info,
		.data_size = sizeof(bool[info->width * info->height]),
		.function = function,
		.color = color
	};
	g.data = malloc(g.data_size);

	return g;
}

void graph_plot(Graph *g)
{
	bool *data = g->data;
	memset(data, 0, g->data_size);

	uint32_t width     = g->info->width;
	vec2     size_half = g->info->size_half;
	double   detail    = g->info->graph_detail;
	vec2     scale     = g->info->graph_scale;
	vec2     offset    = g->info->graph_offset;

	func function = g->function;

	const double x_increment = 1.0 / detail;
	const vec2 size_transformed = {
		size_half.x / scale.x,
		size_half.y
	};

	double y;
	double x_scaled;
	double y_scaled;

	for (double x = -size_transformed.x; x < size_transformed.x; x += x_increment) {
		y = function(x);

		if (isnan(y) || isinf(y))
			continue;

		x_scaled = x * scale.x;
		y_scaled = y * scale.y;

		if (y_scaled >= size_transformed.y || y_scaled <= -size_transformed.y)
			continue;

		data[(int64_t)(x_scaled + size_half.x - 1) + (int64_t)(y_scaled + size_half.y - 1) * width] = true;
	}
}

void graph_destroy(Graph *g)
{
	free(g->data);
	*g = (Graph) { 0 };
}
