#include "pch.h"

#include "GraphicsHelper.h"
#include "Memory.h"

#include <Kore/Graphics1/Graphics.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Math/Core.h>
#include <limits>
#include <string.h>

using namespace Kore;

void clear(float red, float green, float blue) {
	for (int y = 0; y < Graphics1::height(); ++y) {
		for (int x = 0; x < Graphics1::width(); ++x) {
			Graphics1::setPixel(x, y, red, green, blue);
		}
	}
}

void* loadImage(const char* filename, int* imageWidth, int* imageHeight) {
	Graphics1::Image image(filename, true);
	*imageWidth = image.width;
	*imageHeight = image.height;
	void* memory = Memory::allocate(image.width * image.height * 4);
	memcpy(memory, image.data, image.dataSize);
	return memory;
}

int readPixel(Kore::u8* image, int imageWidth, int imageHeight, int x, int y) {
	int c = *(int*)&(image)[imageWidth * 4 * y + x * 4];
	int a = (c >> 24) & 0xff;
	
	int b = (c >> 16) & 0xff;
	int g = (c >> 8) & 0xff;
	int r = c & 0xff;
	
	return a << 24 | r << 16 | g << 8 | b;
}

void drawImage(Kore::u8* image, int imageWidth, int imageHeight, int x, int y) {
	int ystart = max(0, -y);
	int xstart = max(0, -x);
	int h = min(imageHeight, Graphics1::height() - y);
	int w = min(imageWidth, Graphics1::width() - x);
	for (int yy = ystart; yy < h; ++yy) {
		for (int xx = xstart; xx < w; ++xx) {
			int col = readPixel(image, imageWidth, imageHeight, xx, yy);
			
			float a = ((col >> 24) & 0xff) / 255.0f;
			float r = ((col >> 16) & 0xff) / 255.0f;
			float g = ((col >> 8) & 0xff) / 255.0f;
			float b = (col & 0xff) / 255.0f;
			Graphics1::setPixel(x + xx, y + yy, r, g, b);
		}
	}
}

namespace {
	struct Edge {
		int x1, y1, x2, y2;
		
		Edge(int x1, int y1, int x2, int y2) {
			if (y1 < y2) {
				this->x1 = x1;
				this->y1 = y1;
				this->x2 = x2;
				this->y2 = y2;
			}
			else {
				this->x1 = x2;
				this->y1 = y2;
				this->x2 = x1;
				this->y2 = y1;
			}
		}
	};
	
	struct Span {
		int x1, x2;
		
		Span(int x1, int x2) {
			if (x1 < x2) {
				this->x1 = x1;
				this->x2 = x2;
			}
			else {
				this->x1 = x2;
				this->x2 = x1;
			}
		}
	};
	
	void drawSpan(const Span &span, int y) {
		int xdiff = span.x2 - span.x1;
		if (xdiff == 0) return;
		
		float factor = 0.0f;
		float factorStep = 1.0f / xdiff;
		
		for (int x = span.x1; x < span.x2; ++x) {
			//setPixel(x, y, 1, 0, 0);
			Graphics1::setPixel(x, y, 1, 0, 0);
			factor += factorStep;
		}
	}
	
	void drawSpansBetweenEdges(const Edge &e1, const Edge &e2) {
		float e1ydiff = (float)(e1.y2 - e1.y1);
		if (e1ydiff == 0.0f) return;
		
		float e2ydiff = (float)(e2.y2 - e2.y1);
		if (e2ydiff == 0.0f) return;
		
		float e1xdiff = (float)(e1.x2 - e1.x1);
		float e2xdiff = (float)(e2.x2 - e2.x1);
		
		float factor1 = (float)(e2.y1 - e1.y1) / e1ydiff;
		float factorStep1 = 1.0f / e1ydiff;
		float factor2 = 0.0f;
		float factorStep2 = 1.0f / e2ydiff;
		
		for (int y = e2.y1; y < e2.y2; ++y) {
			Span span(e1.x1 + (int)(e1xdiff * factor1), e2.x1 + (int)(e2xdiff * factor2));
			drawSpan(span, y);
			factor1 += factorStep1;
			factor2 += factorStep2;
		}
	}
}

void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	Edge edges[3] = {
		Edge((int)Kore::round(x1), (int)Kore::round(y1), (int)Kore::round(x2), (int)Kore::round(y2)),
		Edge((int)Kore::round(x2), (int)Kore::round(y2), (int)Kore::round(x3), (int)Kore::round(y3)),
		Edge((int)Kore::round(x3), (int)Kore::round(y3), (int)Kore::round(x1), (int)Kore::round(y1))
	};
	
	int maxLength = 0;
	int longEdge = 0;
	
	for (int i = 0; i < 3; ++i) {
		int length = edges[i].y2 - edges[i].y1;
		if (length > maxLength) {
			maxLength = length;
			longEdge = i;
		}
	}
	
	int shortEdge1 = (longEdge + 1) % 3;
	int shortEdge2 = (longEdge + 2) % 3;
	
	drawSpansBetweenEdges(edges[longEdge], edges[shortEdge1]);
	drawSpansBetweenEdges(edges[longEdge], edges[shortEdge2]);
}
