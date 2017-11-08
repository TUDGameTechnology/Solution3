#pragma once

// Clears the screen to a single color
void clear(float red, float green, float blue);

// Loads and returns an image using Memory::allocate.
// Also sets the width and height parameters to the image dimensions.
void* loadImage(const char* filename, int* width, int* height);

// Draws 32bit RGBA pixels (for examples the ones loaded via loadImage)
void drawImage(Kore::u8* image, int imageWidth, int imageHeight, int x, int y);

// Draws a beautiful, red triangle
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
