#ifndef CGRAPH_H
#define CGRAPH_H

#include "types.h"

typedef struct {
	ui32 width, height;
	vec2 size_half;

	f64 graph_detail; // values in between pixels are calculated for increased detail
	vec2 graph_scale; // one unit is 1/graph_scale pixels wide
	vec2 graph_offset;

	bool redraw;
} SessionInfo;

void cgraph_run(ui32 width, ui32 height, ui32 pixel_width);

#endif
