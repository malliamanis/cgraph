#include "cgraph.h"

#define PIXEL_WIDTH 1
#define WIDTH (1280 / PIXEL_WIDTH)
#define HEIGHT (WIDTH * 9 / 16)

int main(void)
{
	cgraph_run(WIDTH, HEIGHT, PIXEL_WIDTH);

	return 0;
}
