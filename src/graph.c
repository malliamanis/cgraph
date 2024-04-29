#include <math.h>
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
	memset(data, 0, g->data_size * sizeof(*data));

	SessionInfo info = *g->info;
	func function = g->function;

	const double x_increment       = 1.0 / info.graph_detail;
	const double width_half_scaled = info.size_half.x / info.graph_scale.x;
	const double offset_x_scaled   = info.graph_offset.x / info.graph_scale.x;

	double y, x_scaled, y_scaled;

	for (double x = -width_half_scaled + 1.0 / info.size_half.x; x < width_half_scaled; x += x_increment) {
		y = function(x - offset_x_scaled);

		if (isnan(y) || isinf(y))
			continue;

		x_scaled = x * info.graph_scale.x;
		y_scaled = y * info.graph_scale.y - info.graph_offset.y;

		if (y_scaled >= info.size_half.y || y_scaled <= -info.size_half.y)
			continue;

		data[(int64_t)(x_scaled + info.size_half.x - 1) + (int64_t)(y_scaled + info.size_half.y - 1) * info.width] = true;
	}
}

void graph_destroy(Graph *g)
{
	free(g->data);
	*g = (Graph) { 0 };
}
