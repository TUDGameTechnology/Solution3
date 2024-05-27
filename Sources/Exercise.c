#include <math.h>
#include <stdbool.h>

#include <kinc/audio1/audio.h>
#include <kinc/graphics1/graphics.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/io/filereader.h>
#include <kinc/math/core.h>
#include <kinc/system.h>

#include "GraphicsHelper.h"
#include "Memory.h"
#include "ObjLoader.h"

#define WIDTH 512
#define HEIGHT 512
#define CAMERA_ROTATION_SPEED_X 0.001
#define CAMERA_ROTATION_SPEED_Y 0.001

double start_time;
float last_t = 0;
struct Mesh* mesh;

const float movement_speed = 2;
float zmin = -1;

float camera_x;
float camera_y;
float camera_z;

float camera_rot_x = 0;
float camera_rot_y = 0;
float camera_rot_z = 0;

bool move_up = false;
bool move_down = false;
bool move_right = false;
bool move_left = false;
bool move_forward = false;
bool move_backward = false;

bool rotate = false;
int mouse_press_x, mouse_press_y;

void init_camera() {
	camera_x = 0;
	camera_y = 0;
	camera_z = 1.5;

	camera_rot_x = 0;
	camera_rot_y = 0;
	camera_rot_z = 0;
}

void rotate3d(float* x, float* y, float* z, float rx, float ry, float rz) {
	float d1x = cos(ry) * *x + sin(ry) * *z;
	float d1y = *y;
	float d1z = cos(ry) * *z - sin(ry) * *x;
	float d2x = d1x;
	float d2y = cos(rx) * d1y - sin(rx) * d1z;
	float d2z = cos(rx) * d1z + sin(rx) * d1y;
	float d3x = cos(rz) * d2x - sin(rz) * d2y;
	float d3y = cos(rz) * d2y + sin(rz) * d2x;
	float d3z = d2z;

	*x = d3x;
	*y = d3y;
	*z = d3z;
}

void update(void* data) {
	float t = (float)(kinc_time() - start_time);

	kinc_g1_begin();
	clear(0.0f, 0.0f, 0.0f);

	float time_since_last_frame = t - last_t;
	last_t = t;

	// Update camera:
	float camera_movement_x = 0;
	float camera_movement_y = 0;
	float camera_movement_z = 0;

	if (move_up)
		camera_movement_y -= time_since_last_frame * movement_speed;
	if (move_down)
		camera_movement_y += time_since_last_frame * movement_speed;
	if (move_left)
		camera_movement_x -= time_since_last_frame * movement_speed;
	if (move_right)
		camera_movement_x += time_since_last_frame * movement_speed;
	if (move_forward)
		camera_movement_z -= time_since_last_frame * movement_speed;
	if (move_backward)
		camera_movement_z += time_since_last_frame * movement_speed;

	// Rotate direction according to current rotation
	rotate3d(&camera_movement_x, &camera_movement_y, &camera_movement_z, -camera_rot_x, 0, -camera_rot_z);
	rotate3d(&camera_movement_x, &camera_movement_y, &camera_movement_z, 0, -camera_rot_y, -camera_rot_z);

	camera_x += camera_movement_x;
	camera_y += camera_movement_y;
	camera_z += camera_movement_z;

	clear(0, 0, 0);

	/************************************************************************/
	/* Exercise 3 Practical Task 1                                          */
	/* Implement camera controls to translate and rotate the mesh           */
	/* Implement perspective projection                                     */
	/************************************************************************/

	for (int i = 0; i < mesh->num_faces; ++i) {
		// Get the indices into the vertices array for each vertex of the mesh
		int i1 = mesh->indices[i * 3 + 0];
		int i2 = mesh->indices[i * 3 + 1];
		int i3 = mesh->indices[i * 3 + 2];

		// Get the x, y, z coordinates of each vertex
		float x1 = mesh->vertices[i1 * 5 + 0];
		float y1 = -mesh->vertices[i1 * 5 + 1];
		float z1 = mesh->vertices[i1 * 5 + 2];

		float x2 = mesh->vertices[i2 * 5 + 0];
		float y2 = -mesh->vertices[i2 * 5 + 1];
		float z2 = mesh->vertices[i2 * 5 + 2];

		float x3 = mesh->vertices[i3 * 5 + 0];
		float y3 = -mesh->vertices[i3 * 5 + 1];
		float z3 = mesh->vertices[i3 * 5 + 2];

		// Move vectors
		x1 -= camera_x;
		y1 -= camera_y;
		z1 -= camera_z;
		x2 -= camera_x;
		y2 -= camera_y;
		z2 -= camera_z;
		x3 -= camera_x;
		y3 -= camera_y;
		z3 -= camera_z;

		// Camera rotation
		rotate3d(&x1, &y1, &z1, camera_rot_x, camera_rot_y, camera_rot_z);
		rotate3d(&x2, &y2, &z2, camera_rot_x, camera_rot_y, camera_rot_z);
		rotate3d(&x3, &y3, &z3, camera_rot_x, camera_rot_y, camera_rot_z);

		if (z1 > zmin || z2 > zmin || z3 > zmin)
			continue;

		// Perspective transform:
		x1 = (zmin / z1) * x1;
		y1 = (zmin / z1) * y1;
		x2 = (zmin / z2) * x2;
		y2 = (zmin / z2) * y2;
		x3 = (zmin / z3) * x3;
		y3 = (zmin / z3) * y3;

		// Draw the triangle
		float draw_scale = 1024;
		draw_triangle(
			x1 * draw_scale + WIDTH / 2, y1 * draw_scale + HEIGHT / 2,
			x2 * draw_scale + WIDTH / 2, y2 * draw_scale + HEIGHT / 2,
			x3 * draw_scale + WIDTH / 2, y3 * draw_scale + HEIGHT / 2);
	}

	kinc_g1_end();
}

void key_down(int code, void* data) {
	/************************************************************************/
	/* Use the keyboard input to control the transformations                */
	/************************************************************************/
	switch (code) {
	case KINC_KEY_LEFT:
	case KINC_KEY_A:
		move_left = true;
		break;
	case KINC_KEY_RIGHT:
	case KINC_KEY_D:
		move_right = true;
		break;
	case KINC_KEY_UP:
		move_up = true;
		break;
	case KINC_KEY_DOWN:
		move_down = true;
		break;
	case KINC_KEY_W:
		move_forward = true;
		break;
	case KINC_KEY_S:
		move_backward = true;
		break;
	case KINC_KEY_R:
		init_camera();
		break;
	case KINC_KEY_L:
		kinc_log(KINC_LOG_LEVEL_INFO, "Position: (%.2f, %.2f, %.2f) - Rotation: (%.2f, %.2f, %.2f)\n", camera_x, camera_y, camera_z, camera_rot_x, camera_rot_y, camera_rot_z);
		break;
	default:
		break;
	}
}

void key_up(int code, void* data) {
	switch (code) {
	case KINC_KEY_LEFT:
	case KINC_KEY_A:
		move_left = false;
		break;
	case KINC_KEY_RIGHT:
	case KINC_KEY_D:
		move_right = false;
		break;
	case KINC_KEY_UP:
		move_up = false;
		break;
	case KINC_KEY_DOWN:
		move_down = false;
		break;
	case KINC_KEY_W:
		move_forward = false;
		break;
	case KINC_KEY_S:
		move_backward = false;
		break;
	default:
		break;
	}
}

void mouse_move(int window, int x, int y, int movementX, int movementY, void* data) {
	if (rotate) {
		camera_rot_x += (float)((mouse_press_y - y) * CAMERA_ROTATION_SPEED_X);
		camera_rot_y -= (float)((mouse_press_x - x) * CAMERA_ROTATION_SPEED_Y);
		mouse_press_x = x;
		mouse_press_y = y;
	}
}

void mouse_press(int window, int button, int x, int y, void* data) {
	rotate = true;
	mouse_press_x = x;
	mouse_press_y = y;
}

void mouse_release(int window, int button, int x, int y, void* data) {
	rotate = false;
}

int kickstart(int argc, char** argv) {
	kinc_init("Solution 3", WIDTH, HEIGHT, NULL, NULL);

	kinc_g1_init(WIDTH, HEIGHT);
	kinc_set_update_callback(update, NULL);

	start_time = kinc_time();

	memory_init();
	mesh = load_obj("bunny.obj");

	kinc_keyboard_set_key_down_callback(key_down, NULL);
	kinc_keyboard_set_key_up_callback(key_up, NULL);
	kinc_mouse_set_move_callback(mouse_move, NULL);
	kinc_mouse_set_press_callback(mouse_press, NULL);
	kinc_mouse_set_release_callback(mouse_release, NULL);

	init_camera();

	kinc_start();

	return 0;
}
