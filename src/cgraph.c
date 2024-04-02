#include <SDL2/SDL.h>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "graph.h"
#include "cgraph.h"

#define TITLE "cgraph"

#define WHITE  0xFFFFFFFF
#define BLACK  0
#define RED    0xFF770000
#define GREEN  0xFF007700
#define BLUE   0xFF000077
#define PURPLE 0xFF770077

#define DEFAULT_SCALE_X 100
#define DEFAULT_SCALE_Y 100
#define DEFAULT_DETAIL 10000

#define ZOOM_FACTOR 1.05

typedef struct {
	bool quit;
	ui32 width, height, size;

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *screen;

	ui32 *pixels;

	SessionInfo info;

	Graph *graphs;
	ui32 graphs_amount;
	bool replot_all;
} State;

static State s = {0};

static f64 factorial(f64 x)
{
	return tgamma(x + 1);
}

static f64 f(f64 x)
{
	return sin(1/x);
}

static f64 g(f64 x)
{
	return NAN;
}

static f64 h(f64 x)
{
	return NAN;
}

static f64 t(f64 x)
{
	return NAN;
}

void cgraph_run(ui32 width, ui32 height, ui32 pixel_width)
{
	// init

	s.width = width;
	s.height = height;
	s.size = width * height;

	s.quit = false;

	SDL_Init(SDL_INIT_VIDEO);

	s.window =
		SDL_CreateWindow(
			TITLE,
			SDL_WINDOWPOS_CENTERED_DISPLAY(0),
			SDL_WINDOWPOS_CENTERED_DISPLAY(0),
			width * pixel_width,
			height * pixel_width,
			SDL_WINDOW_ALLOW_HIGHDPI
		);

	s.renderer = SDL_CreateRenderer(s.window, -1, SDL_RENDERER_PRESENTVSYNC);
	// SDL_RenderSetScale(renderer, pixel_width, pixel_width);

	s.screen =
		SDL_CreateTexture(
			s.renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			width,
			height
		);

	s.pixels = calloc(s.size, sizeof *s.pixels);

	s.info = (SessionInfo) {
		.width = width,
		.height = height,
		.size_half = { width / 2.0, height / 2.0 },
		.graph_detail = DEFAULT_DETAIL,
		.graph_scale = { DEFAULT_SCALE_X, DEFAULT_SCALE_Y },
		.graph_offset = { 0.0, 0.0 },
		.redraw = true
	};

	s.graphs_amount = 4;
	s.graphs =
		(Graph[]) {
			graph_create(&s.info, f, BLUE),
			graph_create(&s.info, g, GREEN),
			graph_create(&s.info, h, RED),
			graph_create(&s.info, t, PURPLE)
		};
	s.replot_all = true;

	ui64 ticks = 60;
	ui64 delta_time = 1000 / ticks;

	ui64 current_time = SDL_GetTicks64();
	ui64 new_time;
	ui64 accumulator = 0;

	while (!s.quit) {
		// update

		SDL_PumpEvents();

		const ui8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_ESCAPE])
			s.quit = true;
		if (keys[SDL_SCANCODE_UP]) {
			s.info.graph_scale.x *= ZOOM_FACTOR;
			s.info.graph_scale.y *= ZOOM_FACTOR;
			s.info.graph_detail  *= ZOOM_FACTOR;

			s.replot_all = true;
		}
		if (keys[SDL_SCANCODE_DOWN]) {
			s.info.graph_scale.x /= ZOOM_FACTOR;
			s.info.graph_scale.y /= ZOOM_FACTOR;
			s.info.graph_detail  /= ZOOM_FACTOR;

			s.replot_all = true;
		}

		new_time = SDL_GetTicks64();

		accumulator += new_time - current_time;
		current_time = new_time;

		while (accumulator >= delta_time) {
			accumulator -= delta_time;
			
			// tick
			if (s.replot_all) {
				for (int i = 0; i < s.graphs_amount; ++i)
					graph_plot(&s.graphs[i]);

				s.replot_all = false;
			}
		}

		// render

		if (s.info.redraw) {
			memset(s.pixels, WHITE, s.size * (sizeof *s.pixels));

			for (ui32 i = 0; i < width; ++i) {
				if (i < height) {
					// vertical axis
					s.pixels[(ui32)(s.info.size_half.x - 1) + i * width] = BLACK;
					s.pixels[(ui32)(s.info.size_half.x) + i * width] = BLACK;
				}

				// horizontal axis
				s.pixels[i + (ui32)(s.info.size_half.y - 1) * width] = BLACK;
				s.pixels[i + (ui32)(s.info.size_half.y) * width] = BLACK;
			}

			for (ui32 i = 0; i < s.graphs_amount; ++i) {
				bool *data = s.graphs[i].data;
				ui32 color = s.graphs[i].color;

				for (ui32 y = 0; y < height; ++y) {
					for (ui32 x = 0; x < width; ++x) {
						if (data[x + y * width]) {
							s.pixels[x + y * width] = color;

							// make it thicker
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

			s.info.redraw = false;
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
