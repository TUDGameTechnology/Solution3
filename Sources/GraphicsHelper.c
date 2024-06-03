#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include <kinc/graphics1/graphics.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/image.h>
#include <kinc/math/core.h>

#include "GraphicsHelper.h"
#include "Memory.h"

void clear(float red, float green, float blue) {
	for (int y = 0; y < kinc_g1_height(); ++y) {
		for (int x = 0; x < kinc_g1_width(); ++x) {
			kinc_g1_set_pixel(x, y, red, green, blue);
		}
	}
}

int read_pixel(uint8_t* image, int image_width, int image_height, int x, int y) {
	uint32_t c = image[image_width * 4 * y + x * 4];
	int a = (c >> 24) & 0xff;

	int b = (c >> 16) & 0xff;
	int g = (c >> 8) & 0xff;
	int r = c & 0xff;

	return a << 24 | r << 16 | g << 8 | b;
}

void draw_image(uint8_t* image, int image_width, int image_height, int x, int y) {
	int ystart = max(0, -y);
	int xstart = max(0, -x);
	int h = min(image_height, kinc_g1_height() - y);
	int w = min(image_width, kinc_g1_width() - x);
	for (int yy = ystart; yy < h; ++yy) {
		for (int xx = xstart; xx < w; ++xx) {
			int col = read_pixel(image, image_width, image_height, xx, yy);

			float a = ((col >> 24) & 0xff) / 255.0f;
			float r = ((col >> 16) & 0xff) / 255.0f;
			float g = ((col >> 8)  & 0xff) / 255.0f;
			float b = (col & 0xff) / 255.0f;
			kinc_g1_set_pixel(x + xx, y + yy, r, g, b);
		}
	}
}

struct Edge {
	int x1, y1, x2, y2;
};

struct Edge new_edge(int x1, int y1, int x2, int y2) {
	struct Edge edge;
	if (y1 < y2) {
		edge.x1 = x1;
		edge.y1 = y1;
		edge.x2 = x2;
		edge.y2 = y2;
	} else {
		edge.x1 = x2;
		edge.y1 = y2;
		edge.x2 = x1;
		edge.y2 = y1;
	}
	return edge;
}

struct Span {
	int x1, x2;
};

struct Span new_span(int x1, int x2) {
	struct Span span;
	if (x1 < x2) {
		span.x1 = x1;
		span.x2 = x2;
	} else {
		span.x1 = x2;
		span.x2 = x1;
	}
	return span;
}

void draw_span(const struct Span span, int y) {
	int xdiff = span.x2 - span.x1;
	if (xdiff == 0) return;

	float factor = 0.0f;
	float factor_step = 1.0f / xdiff;

	int x1 = max(0, span.x1);
	int x2 = min(span.x2, kinc_g1_width());
	if (y < 0 || y >= kinc_g1_height()) return;
	for (int x = x1; x < x2; ++x) {
		kinc_g1_set_pixel(x, y, 1, 0, 0);
		factor += factor_step;
	}
}

void draw_spans_between_edges(const struct Edge e1, const struct Edge e2) {
	float e1ydiff = (float)(e1.y2 - e1.y1);
	if (e1ydiff == 0.0f) return;

	float e2ydiff = (float)(e2.y2 - e2.y1);
	if (e2ydiff == 0.0f) return;

	float e1xdiff = (float)(e1.x2 - e1.x1);
	float e2xdiff = (float)(e2.x2 - e2.x1);

	float factor_1 = (float)(e2.y1 - e1.y1) / e1ydiff;
	float factor_step_1 = 1.0f / e1ydiff;
	float factor_2 = 0.0f;
	float factor_step_2 = 1.0f / e2ydiff;

	for (int y = e2.y1; y < e2.y2; ++y) {
		struct Span span = new_span(e1.x1 + (int)(e1xdiff * factor_1), e2.x1 + (int)(e2xdiff * factor_2));
		draw_span(span, y);
		factor_1 += factor_step_1;
		factor_2 += factor_step_2;
	}
}

void draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	struct Edge edges[3] = {
		new_edge((int)round(x1), (int)round(y1), (int)round(x2), (int)round(y2)),
		new_edge((int)round(x2), (int)round(y2), (int)round(x3), (int)round(y3)),
		new_edge((int)round(x3), (int)round(y3), (int)round(x1), (int)round(y1))
	};

	int max_ength = 0;
	int long_edge = 0;

	for (int i = 0; i < 3; ++i) {
		int length = edges[i].y2 - edges[i].y1;
		if (length > max_ength) {
			max_ength = length;
			long_edge = i;
		}
	}

	int short_edge_1 = (long_edge + 1) % 3;
	int short_edge_2 = (long_edge + 2) % 3;

	draw_spans_between_edges(edges[long_edge], edges[short_edge_1]);
	draw_spans_between_edges(edges[long_edge], edges[short_edge_2]);
}
