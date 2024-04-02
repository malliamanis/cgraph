#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"

Graph graph_create(SessionInfo *info, func function, ui32 color)
{
	Graph g;

	g.info = info;
	g.data_size = info->width * info->height * (sizeof *g.data);
	g.data = malloc(g.data_size);
	g.function = function;
	g.color = color;

	return g;
}

void graph_plot(Graph *g)
{
	bool *data = g->data;
	memset(data, 0, g->data_size);

	ui32 width     = g->info->width;
	vec2 size_half = g->info->size_half;
	f64  detail    = g->info->graph_detail;
	vec2 scale     = g->info->graph_scale;
	vec2 offset    = g->info->graph_offset;

	func function = g->function;

	const f64 x_increment = 1.0 / detail;
	const f64 width_half_transformed = (size_half.x + offset.x) / scale.x;
	const f64 height_half_transformed = size_half.y + offset.y;

	f64 y;
	f64 x_scaled;
	f64 y_scaled;

	for (f64 x = -width_half_transformed; x < width_half_transformed; x += x_increment) {
		y = function(x);

		if (isnan(y) || isinf(y))
			continue;

		x_scaled = x * scale.x;
		y_scaled = y * scale.y;

		if (y_scaled >= height_half_transformed || y_scaled <= -height_half_transformed)
			continue;

		data[(i64)(x_scaled + size_half.x - 1) + (i64)(y_scaled + size_half.y - 1) * width] = true;
	}

	g->info->redraw = true;
}

void graph_destroy(Graph *g)
{
	free(g->data);
	memset(g, 0, sizeof *g);
}
