#include <SDL2/SDL.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cgraph.h"

#define TITLE "cgraph"

#define WHITE 0xFFFFFFFF
#define BLUE 0xFF000077
#define BLACK 0

typedef struct {
	bool *data;
	ui32 color;
} Graph;

void cgraph_run(ui32 width, ui32 height, ui32 scale)
{
	// init
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window =
		SDL_CreateWindow(
			TITLE,
			SDL_WINDOWPOS_CENTERED_DISPLAY(0),
			SDL_WINDOWPOS_CENTERED_DISPLAY(0),
			width * scale,
			height * scale,
			SDL_WINDOW_ALLOW_HIGHDPI);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	// SDL_RenderSetScale(renderer, scale, scale);

	SDL_Texture *screen =
		SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			width,
			height);


	const ui32 size = width * height;

	const double width_half = width / 2.0;
	const double height_half = height / 2.0;

	bool quit = false;

	bool redraw = true;
	ui32 *pixels = calloc(size, sizeof(ui32));

	ui32 graphs_amount = 1;
	Graph graphs[] = {{calloc(size, sizeof(bool)), BLUE}};

	const ui32 graph_scale = 25; // one pixel is 1/graph_scale square units
	const ui32 detail = (200 * 200 / graph_scale) + 1; // values in between pixels are calculated for increased detail

	for (int i = 0; i < graphs_amount; ++i) {
		double y;
		double x_scaled;
		double y_scaled;
		bool *data = graphs[i].data;

		for (double x = -(width_half - 1); x < width_half; x += 1.0 / (graph_scale * detail)) {
			/** FUNCTION **/
			y = x * cos(x*x/ 10);

			x_scaled = x * graph_scale;
			y_scaled = y * graph_scale;

			if (y_scaled >= height_half || y_scaled <= -height_half || x_scaled >= width_half || x_scaled <= -width_half)
				continue;

			data[(int)(x_scaled + width_half - 1) + (int)(y_scaled + height_half - 1) * width] = true;
		}
	}

	while (!quit) {
		// update

		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					quit = true;
				case SDL_KEYDOWN:
					if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
						quit = true;
					break;
			}
		}

		// render

		if (redraw) {
			redraw = false;

			for (int x = 0; x < width; ++x) {
				for (int y = 0; y < height; ++y) {
					if (x == width_half - 1/* || y == height_half - 1*/)
						pixels[x + (int)(height_half - 1) * width] = BLACK;
					else
						pixels[x + y * width] = WHITE;
				}

				pixels[x + (int)(height_half - 1) * width] = BLACK;
			}

			for (int i = 0; i < graphs_amount; ++i) {
				bool *data = graphs[i].data;
				int color = graphs[i].color;

				for (int y = 0; y < height; ++y) {
					for (int x = 0; x < width; ++x) {
						if (data[x + y * width]) {
							pixels[x + y * width] = color;
							if (x < width - 1)
								pixels[x + 1 + y * width] = color;
							if (x > 0)
								pixels[x - 1 + y * width] = color;

							if (y < height - 1)
								pixels[x + (y + 1) * width] = color;
							if (y > 0)
								pixels[x + (y - 1) * width] = color;
						}
					}
				}
			}

			SDL_UpdateTexture(screen, NULL, pixels, width * (sizeof *pixels));
		}

		SDL_RenderCopyEx(renderer, screen, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
		SDL_RenderPresent(renderer);
	}

	
	// quit

	for (int i = 0; i < graphs_amount; ++i)
		free(graphs[i].data);
	free(pixels);

	SDL_DestroyTexture(screen);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	SDL_Quit();
}
