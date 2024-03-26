#include "cgraph.h"

#define SCALE 1
#define WIDTH (1280 / SCALE)
#define HEIGHT (WIDTH * 9 / 16)

int main(void)
{
	cgraph_run(WIDTH, HEIGHT, SCALE);

	return 0;
}
