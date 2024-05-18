#ifndef CGRAPH_H
#define CGRAPH_H

#include <stdint.h>
#include <stdbool.h>

#include "types.h"

typedef struct {
	uint32_t width, height;
	vec2 size_half;

	double graph_detail; // values in between pixels are calculated for increased detail
	vec2 graph_scale; // one unit is 1/graph_scale pixels wide
	vec2 graph_offset;
} SessionInfo;

void cgraph_run(uint32_t window_width, uint32_t window_height, uint32_t pixel_width);

#endif
