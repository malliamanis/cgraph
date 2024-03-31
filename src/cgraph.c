#include <SDL2/SDL.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cgraph.h"

#define TITLE "cgraph"

#define WHITE 0xFFFFFFFF
#define BLUE  0xFF000077
#define GREEN 0xFF007700
#define BLACK 0

#define DEFAULT_SCALE_X 25
#define DEFAULT_SCALE_Y 25
#define DEFAULT_DETAIL 5000

#define ZOOM_FACTOR 1.1

typedef f64 (*func)(f64);

typedef struct {
	bool *data;
	ui32 color;

	func function;
} Graph;

typedef struct {
	bool quit;
	ui32 width, height;
	f64 width_half, height_half;
	ui32 size;

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *screen;

	ui32 *pixels;
	bool redraw;

	Graph *graphs;
	ui32 graphs_amount;
	bool replot_all;

	double graph_scale_x, graph_scale_y;
	double detail;
} State;

static State s = {0};

static f64 f(f64 x)
{
	return 1/sin(x);
}

static f64 lin(f64 x)
{
	return 1/cos(x);
}

static void plot(ui32 graph_index);

void cgraph_run(ui32 width, ui32 height, ui32 pixel_width)
{
	// init

	s.width = width;
	s.height = height;
	s.size = width * height;

	s.width_half = width / 2.0;
	s.height_half = height / 2.0;

	s.quit = false;

	SDL_Init(SDL_INIT_VIDEO);

	s.window =
		SDL_CreateWindow(
			TITLE,
			SDL_WINDOWPOS_CENTERED_DISPLAY(0),
			SDL_WINDOWPOS_CENTERED_DISPLAY(0),
			width * pixel_width,
			height * pixel_width,
			SDL_WINDOW_ALLOW_HIGHDPI);

	s.renderer = SDL_CreateRenderer(s.window, -1, SDL_RENDERER_PRESENTVSYNC);
	// SDL_RenderSetScale(renderer, pixel_width, pixel_width);

	s.screen =
		SDL_CreateTexture(
			s.renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			width,
			height);


	s.pixels = calloc(s.size, sizeof *s.pixels);
	s.redraw = true;

	s.graphs_amount = 2;
	s.graphs = 
		(Graph[]) {
			{ calloc(s.size, sizeof *(s.graphs[0].data)), BLUE,  f },
			{ calloc(s.size, sizeof *(s.graphs[0].data)), GREEN, lin }
		};

	s.replot_all = true;

	// one unit is 1/graph_scale pixels wide
	s.graph_scale_x = DEFAULT_SCALE_X;
	s.graph_scale_y = DEFAULT_SCALE_Y;

	s.detail = DEFAULT_DETAIL; // values in between pixels are calculated for increased detail


	while (!s.quit) {
		// update

		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					s.quit = true;
				case SDL_KEYDOWN:
					switch(event.key.keysym.scancode) {
						case SDL_SCANCODE_ESCAPE:
							s.quit = true;
							break;
						case SDL_SCANCODE_UP:
							s.graph_scale_x *= ZOOM_FACTOR;
							s.graph_scale_y *= ZOOM_FACTOR;
							s.detail *= ZOOM_FACTOR;

							s.replot_all = true;
							break;
						case SDL_SCANCODE_DOWN:
							s.graph_scale_x *= 1 / ZOOM_FACTOR;
							s.graph_scale_y *= 1 / ZOOM_FACTOR;
							s.detail *= 1 / ZOOM_FACTOR;
							
							s.replot_all = true;
							break;

						default:
							break;
					}
					break;
			}
		}

		if (s.replot_all) {
			for (int i = 0; i < s.graphs_amount; ++i)
				plot(i);

			s.replot_all = false;
		}

		// render

		if (s.redraw) {
			for (ui32 x = 0; x < width; ++x) {
				for (ui32 y = 0; y < height; ++y) {
					if (x == s.width_half - 1/* || y == height_half - 1*/)
						s.pixels[x + (ui32)(s.height_half - 1) * width] = BLACK;
					else
						s.pixels[x + y * width] = WHITE;
				}

				s.pixels[x + (ui32)(s.height_half - 1) * width] = BLACK;
			}

			for (ui32 i = 0; i < s.graphs_amount; ++i) {
				bool *data = s.graphs[i].data;
				ui32 color = s.graphs[i].color;

				for (ui32 y = 0; y < height; ++y) {
					for (ui32 x = 0; x < width; ++x) {
						if (data[x + y * width]) {
							s.pixels[x + y * width] = color;

							if (x < width - 1)
								s.pixels[x + 1 + y * width] = color;
							if (x > 0)
								s.pixels[x - 1 + y * width] = color;

							if (y < height - 1)
								s.pixels[x + (y + 1) * width] = color;
							if (y > 0)
								s.pixels[x + (y - 1) * width] = color;
						}
					}
				}
			}

			SDL_UpdateTexture(s.screen, NULL, s.pixels, width * (sizeof *s.pixels));

			s.redraw = false;
		}

		SDL_RenderCopyEx(s.renderer, s.screen, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
		SDL_RenderPresent(s.renderer);
	}

	
	// quit

	for (int i = 0; i < s.graphs_amount; ++i)
		free(s.graphs[i].data);
	free(s.pixels);

	SDL_DestroyTexture(s.screen);
	SDL_DestroyWindow(s.window);
	SDL_DestroyRenderer(s.renderer);

	SDL_Quit();
}

static void plot(ui32 graph_index)
{
	f64 y;
	f64 x_scaled;
	f64 y_scaled;

	bool *data = s.graphs[graph_index].data;
	memset(data, 0, s.size * (sizeof *data));

	func function = s.graphs[graph_index].function;

	const f64 x_increment = 1.0 / s.detail;
	const f64 width_half_scaled = s.width_half / s.graph_scale_x;

	for (f64 x = -width_half_scaled; x < width_half_scaled; x += x_increment) {
		y = function(x);

		if (isnan(y))
			continue;

		x_scaled = x * s.graph_scale_x;
		y_scaled = y * s.graph_scale_y;

		if (y_scaled >= s.height_half || y_scaled <= -s.height_half)
			continue;

		data[(i32)(x_scaled + s.width_half - 1) + (i32)(y_scaled + s.height_half - 1) * s.width] = true;
	}

	s.redraw = true;
}
