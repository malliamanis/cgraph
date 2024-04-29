#include <SDL2/SDL.h>

#include <SDL2/SDL_events.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "graph.h"
#include "cgraph.h"

#define TITLE "cgraph"

#define BLACK  0
#define WHITE  0xFFFFFFFF
#define RED    0xFFFF0000
#define GREEN  0xFF007700
#define BLUE   0xFF000077
#define PURPLE 0xFF770077

#define DEFAULT_SCALE_X  100
#define DEFAULT_SCALE_Y  100
#define DEFAULT_DETAIL   10000
#define DEFAULT_OFFSET_X 0
#define DEFAULT_OFFSET_Y 0

#define DEFAULT_ZOOM_FACTOR 1.1

static double f(double x)
{
	return x * exp(sin(x));
}

static double g(double x)
{
	return NAN;
}

static double h(double x)
{
	return NAN;
}

static double t(double x)
{
	return NAN;
}

void cgraph_run(uint32_t width, uint32_t height, uint32_t pixel_width)
{
	/* INIT */

	size_t size = width * height;
	bool quit = false;

	SessionInfo info = (SessionInfo) {
		.width        =   width,
		.height       =   height,
		.size_half    = { width / 2.0, height / 2.0 },
		.graph_detail =   DEFAULT_DETAIL,
		.graph_scale  = { DEFAULT_SCALE_X, DEFAULT_SCALE_Y },
		.graph_offset = { DEFAULT_OFFSET_X, DEFAULT_OFFSET_Y }
	};

	uint32_t graphs_amount = 4;
	Graph graphs[] = {
		graph_create(&info, f, BLUE),
		graph_create(&info, g, GREEN),
		graph_create(&info, h, RED),
		graph_create(&info, t, PURPLE)
	};
	bool replot_all = true;

	uint32_t *pixels = calloc(width * height, sizeof *pixels);
	bool redraw = true;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow(
		TITLE,
		SDL_WINDOWPOS_CENTERED_DISPLAY(0),
		SDL_WINDOWPOS_CENTERED_DISPLAY(0),
		width * pixel_width,
		height * pixel_width,
		SDL_WINDOW_ALLOW_HIGHDPI
	);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	// SDL_RenderSetScale(renderer, pixel_width, pixel_width);

	SDL_Texture *screen = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		width,
		height
	);

	bool mouse_left_down = false;
	int32_t mouse_diff_x, mouse_diff_y;
	int32_t mouse_scroll_amount;

	uint64_t ticks = 60;
	uint64_t delta_time = 1000 / ticks;

	uint64_t current_time = SDL_GetTicks64();
	uint64_t new_time;
	uint64_t accumulator = 0;

	while (!quit) {
		/* UPDATE */

		mouse_scroll_amount = 0;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_MOUSEWHEEL:
					mouse_scroll_amount = event.wheel.y;
					break;
			}
		}

		const uint8_t *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_ESCAPE])
			quit = true;

		if (keys[SDL_SCANCODE_UP] || mouse_scroll_amount > 0) {
			info.graph_scale.x *= DEFAULT_ZOOM_FACTOR;
			info.graph_scale.y *= DEFAULT_ZOOM_FACTOR;
			info.graph_detail  *= DEFAULT_ZOOM_FACTOR;

			replot_all = true;
		}
		if (keys[SDL_SCANCODE_DOWN] || mouse_scroll_amount < 0) {
			info.graph_scale.x /= DEFAULT_ZOOM_FACTOR;
			info.graph_scale.y /= DEFAULT_ZOOM_FACTOR;
			info.graph_detail  /= DEFAULT_ZOOM_FACTOR;

			replot_all = true;
		}

		if (keys[SDL_SCANCODE_SPACE]) {
			info.graph_detail = DEFAULT_DETAIL;
			info.graph_scale  = (vec2) { DEFAULT_SCALE_X,  DEFAULT_SCALE_Y };
			info.graph_offset = (vec2) { DEFAULT_OFFSET_Y, DEFAULT_OFFSET_Y };

			replot_all = true;
		}

		if (SDL_GetRelativeMouseState(&mouse_diff_x, &mouse_diff_y) & SDL_BUTTON_LMASK) {
			if (mouse_left_down) {
				info.graph_offset.x += (double)mouse_diff_x / pixel_width;
				info.graph_offset.y += (double)mouse_diff_y / pixel_width;

				replot_all = true;
			}

			mouse_left_down = true;
		}
		else
			mouse_left_down = false;

		new_time = SDL_GetTicks64();

		accumulator += new_time - current_time;
		current_time = new_time;

		while (accumulator >= delta_time) {
			accumulator -= delta_time;
			
			/* TICK */
			if (replot_all) {
				for (uint32_t i = 0; i < graphs_amount; ++i)
					graph_plot(&graphs[i]);

				redraw = true;
				replot_all = false;
			}
		}

		/* RENDER */

		if (redraw) {
			memset(pixels, WHITE, size * sizeof(*pixels));

			int32_t width_half_transformed  = info.size_half.x + info.graph_offset.x;
			int32_t height_half_transformed = info.size_half.y - info.graph_offset.y;
			for (uint32_t i = 0; i < width; ++i) {
				// vertical axis
				if (i < height && width_half_transformed > 1 && width_half_transformed < (int32_t)info.width) {
					pixels[(width_half_transformed - 1) + i * width] = BLACK;
					pixels[(width_half_transformed - 2) + i * width] = BLACK;
				}

				// horizontal axis
				if (height_half_transformed > 1 && height_half_transformed < (int32_t)info.height) {
					pixels[i + (height_half_transformed - 1) * width] = BLACK;
					pixels[i + (height_half_transformed - 2) * width] = BLACK;
				}
			}

			for (uint32_t i = 0; i < graphs_amount; ++i) {
				bool *data = graphs[i].data;
				uint32_t color = graphs[i].color;

				for (uint32_t y = 0; y < height; ++y) {
					for (uint32_t x = 0; x < width; ++x) {
						if (data[x + y * width]) {
							pixels[x + y * width] = color;

							// make it thicker
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

			SDL_UpdateTexture(screen, NULL, pixels, width * sizeof(*pixels));

			redraw = false;
		}

		SDL_RenderCopyEx(renderer, screen, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
		SDL_RenderPresent(renderer);
	}

	
	/* QUIT */

	for (uint32_t i = 0; i < graphs_amount; ++i)
		graph_destroy(&graphs[i]);

	free(pixels);

	SDL_DestroyTexture(screen);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	SDL_Quit();
}
