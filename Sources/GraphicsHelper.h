#pragma once

namespace Kore {
	namespace Graphics4 {
		class Texture;
	}
}

void clear(float red, float green, float blue);
Kore::Graphics4::Texture* loadTexture(const char* filename);
void drawTexture(Kore::Graphics4::Texture* image, int x, int y);
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
