#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Audio1/Audio.h>
#include <Kore/Graphics1/Graphics.h>
#include <Kore/Log.h>
#include "GraphicsHelper.h"
#include "ObjLoader.h"

const int width = 512;
const int height = 512;

using namespace Kore;

#define CAMERA_ROTATION_SPEED_X 0.001
#define CAMERA_ROTATION_SPEED_Y 0.001

namespace {
	double startTime;
	float lastT = 0;
	Mesh* mesh;

	const float movementSpeed = 2;
	float zmin = -1;

	float cameraX;
	float cameraY;
	float cameraZ;

	float cameraRotX = 0;
	float cameraRotY = 0;
	float cameraRotZ = 0;

	bool moveUp = false;
	bool moveDown = false;
	bool moveRight = false;
	bool moveLeft = false;
	bool moveForward = false;
	bool moveBackward = false;

	bool rotate = false;
	int mousePressX, mousePressY;

	void initCamera() {
		cameraX = 0;
		cameraY = 0;
		cameraZ = 10;

		cameraRotX = 0;
		cameraRotY = 0;
		cameraRotZ = 0;
	}

	void rotate3d(float &x, float &y, float &z, float rx, float ry, float rz) {
		float d1x = cos(ry) * x + sin(ry) * z;
		float d1y = y;
		float d1z = cos(ry) * z - sin(ry) * x;
		float d2x = d1x;
		float d2y = cos(rx) * d1y - sin(rx) * d1z;
		float d2z = cos(rx) * d1z + sin(rx) * d1y;
		float d3x = cos(rz) * d2x - sin(rz) * d2y;
		float d3y = cos(rz) * d2y + sin(rz) * d2x;
		float d3z = d2z;

		x = d3x;
		y = d3y;
		z = d3z;
	}

	void update()
	{
		float t = (float)(System::time() - startTime);
		Kore::Audio2::update();

		Graphics1::begin();
		clear(0.0f, 0.0f, 0.0f);

		float timeSinceLastFrame = t - lastT;
		lastT = t;

		// Update camera:
		float cameraMovementX = 0;
		float cameraMovementY = 0;
		float cameraMovementZ = 0;

		if (moveUp)
			cameraMovementY -= timeSinceLastFrame * movementSpeed;
		if (moveDown)
			cameraMovementY += timeSinceLastFrame * movementSpeed;
		if (moveLeft)
			cameraMovementX -= timeSinceLastFrame * movementSpeed;
		if (moveRight)
			cameraMovementX += timeSinceLastFrame * movementSpeed;
		if (moveForward)
			cameraMovementZ -= timeSinceLastFrame * movementSpeed;
		if (moveBackward)
			cameraMovementZ += timeSinceLastFrame * movementSpeed;

		// Rotate direction according to current rotation
		rotate3d(cameraMovementX, cameraMovementY, cameraMovementZ, -cameraRotX, 0, -cameraRotZ);
		rotate3d(cameraMovementX, cameraMovementY, cameraMovementZ, 0, -cameraRotY, -cameraRotZ);

		cameraX += cameraMovementX;
		cameraY += cameraMovementY;
		cameraZ += cameraMovementZ;

		clear(0, 0, 0);

		/************************************************************************/
		/* Exercise 3 Practical Task 1                                          */
		/* Implement camera controls to translate and rotate the mesh           */
		/* Implement perspective projection                                     */
		/************************************************************************/

		for (int i = 0; i < mesh->numFaces; ++i) {
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
			x1 -= cameraX;
			y1 -= cameraY;
			z1 -= cameraZ;
			x2 -= cameraX;
			y2 -= cameraY;
			z2 -= cameraZ;
			x3 -= cameraX;
			y3 -= cameraY;
			z3 -= cameraZ;

			// Camera rotation
			rotate3d(x1, y1, z1, cameraRotX, cameraRotY, cameraRotZ);
			rotate3d(x2, y2, z2, cameraRotX, cameraRotY, cameraRotZ);
			rotate3d(x3, y3, z3, cameraRotX, cameraRotY, cameraRotZ);

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
			float drawScale = 1024;
			drawTriangle(
				x1 * drawScale + width / 2, y1 * drawScale + height / 2,
				x2 * drawScale + width / 2, y2 * drawScale + height / 2,
				x3 * drawScale + width / 2, y3 * drawScale + height / 2);
		}

		Graphics1::end();
	}

	void keyDown(KeyCode code) {
		/************************************************************************/
		/* Use the keyboard input to control the transformations                */
		/************************************************************************/
		switch (code)
		{
		case KeyLeft:
		case KeyA:
			moveLeft = true;
			break;
		case KeyRight:
		case KeyD:
			moveRight = true;
			break;
		case KeyUp:
			moveUp = true;
			break;
		case KeyDown:
			moveDown = true;
			break;
		case KeyW:
			moveForward = true;
			break;
		case KeyS:
			moveBackward = true;
			break;
		case KeyR:
			initCamera();
			break;
		case KeyL:
			Kore::log(Kore::LogLevel::Info, "Position: (%.2f, %.2f, %.2f) - Rotation: (%.2f, %.2f, %.2f)\n", cameraX, cameraY, cameraZ, cameraRotX, cameraRotY, cameraRotZ);
			break;
		default:
			break;
		}
	}

	void keyUp(KeyCode code) {
		switch (code)
		{
		case KeyLeft:
		case KeyA:
			moveLeft = false;
			break;
		case KeyRight:
		case KeyD:
			moveRight = false;
			break;
		case KeyUp:
			moveUp = false;
			break;
		case KeyDown:
			moveDown = false;
			break;
		case KeyW:
			moveForward = false;
			break;
		case KeyS:
			moveBackward = false;
			break;
		default:
			break;
		}
	}

	void mouseMove(int window, int x, int y, int movementX, int movementY) {
		if (rotate) {
			cameraRotX += (float)((mousePressY - y) * CAMERA_ROTATION_SPEED_X);
			cameraRotY -= (float)((mousePressX - x) * CAMERA_ROTATION_SPEED_Y);
			mousePressX = x;
			mousePressY = y;
		}
	}

	void mousePress(int window, int button, int x, int y) {
		rotate = true;
		mousePressX = x;
		mousePressY = y;
	}

	void mouseRelease(int window, int button, int x, int y) {
		rotate = false;
	}
}

int kore(int argc, char** argv) {
	System::init("Solution 3", width, height);

	Graphics1::init(width, height);
	Kore::System::setCallback(update);

	startTime = System::time();
	
	Kore::Audio2::init();
	Kore::Audio1::init();

	mesh = loadObj("bunny.obj");

	for (int i = 0; i < mesh->numVertices; i++) {
		mesh->vertices[i * 5 + 0] *= 10;
		mesh->vertices[i * 5 + 1] *= 10;
		mesh->vertices[i * 5 + 2] *= 10;
	}

	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->Move = mouseMove;
	Mouse::the()->Press = mousePress;
	Mouse::the()->Release = mouseRelease;

	initCamera();

	Kore::System::start();

	return 0;
}
