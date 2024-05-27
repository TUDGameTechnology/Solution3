#pragma once

#include <stdint.h>

// Clears the screen to a single color
void clear(float red, float green, float blue);

// Draws 32-bit RGBA pixels (for examples the ones loaded via kinc_image_init_from_file())
void draw_image(uint8_t* image, int image_width, int image_height, int x, int y);

// Draws a beautiful, red triangle
void draw_triangle(float x1, float y1, float x2, float y2, float x3, float y3);
