#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Audio/Mixer.h>
#include <Kore/Log.h>
#include "SimpleGraphics.h"
#include "ObjLoader.h"

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

	void drawBunny(){
		float t = (float)(System::time() - startTime);

		float timeSinceLastFrame = t - lastT;
		lastT = t;

		//update camera:
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

		//rotate direction according to current rotation
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
			int i1 = mesh->indices[i * 3 + 0];
			int i2 = mesh->indices[i * 3 + 1];
			int i3 = mesh->indices[i * 3 + 2];

			float x1 = mesh->vertices[i1 * 5 + 0];
			float y1 = -mesh->vertices[i1 * 5 + 1];
			float z1 = mesh->vertices[i1 * 5 + 2];

			float x2 = mesh->vertices[i2 * 5 + 0];
			float y2 = -mesh->vertices[i2 * 5 + 1];
			float z2 = mesh->vertices[i2 * 5 + 2];

			float x3 = mesh->vertices[i3 * 5 + 0];
			float y3 = -mesh->vertices[i3 * 5 + 1];
			float z3 = mesh->vertices[i3 * 5 + 2];

			//move vectors
			x1 -= cameraX;
			y1 -= cameraY;
			z1 -= cameraZ;
			x2 -= cameraX;
			y2 -= cameraY;
			z2 -= cameraZ;
			x3 -= cameraX;
			y3 -= cameraY;
			z3 -= cameraZ;

			//camera rotation
			rotate3d(x1, y1, z1, cameraRotX, cameraRotY, cameraRotZ);
			rotate3d(x2, y2, z2, cameraRotX, cameraRotY, cameraRotZ);
			rotate3d(x3, y3, z3, cameraRotX, cameraRotY, cameraRotZ);

			if (z1 > zmin || z2 > zmin || z3 > zmin)
				continue;

			//perspective transform:
			x1 = (zmin / z1) * x1;
			y1 = (zmin / z1) * y1;
			x2 = (zmin / z2) * x2;
			y2 = (zmin / z2) * y2;
			x3 = (zmin / z3) * x3;
			y3 = (zmin / z3) * y3;

			drawTriangle(
				x1 * height + width / 2, y1 * height + height / 2,
				x2 * height + width / 2, y2 * height + height / 2,
				x3 * height + width / 2, y3 * height + height / 2);
		}
	}



	void update() {
		Kore::Audio::update();
		startFrame();

		drawBunny();

		endFrame();
	}

	void keyDown(KeyCode code, wchar_t character) {
		switch (code)
		{
		case Key_Left:
		case Key_A:
			moveLeft = true;
			break;
		case Key_Right:
		case Key_D:
			moveRight = true;
			break;
		case Key_Up:
			moveUp = true;
			break;
		case Key_Down:
			moveDown = true;
			break;
		case Key_W:
			moveForward = true;
			break;
		case Key_S:
			moveBackward = true;
			break;
		case Key_R:
			initCamera();
			break;
		case Key_L:
			Kore::log(Kore::LogLevel::Info, "Position: (%.2f, %.2f, %.2f) - Rotation: (%.2f, %.2f, %.2f)\n", cameraX, cameraY, cameraZ, cameraRotX, cameraRotY, cameraRotZ);
			break;
		default:
			break;
		}
	}

	void keyUp(KeyCode code, wchar_t character) {
		switch (code)
		{
		case Key_Left:
		case Key_A:
			moveLeft = false;
			break;
		case Key_Right:
		case Key_D:
			moveRight = false;
			break;
		case Key_Up:
			moveUp = false;
			break;
		case Key_Down:
			moveDown = false;
			break;
		case Key_W:
			moveForward = false;
			break;
		case Key_S:
			moveBackward = false;
			break;
		default:
			break;
		}
	}

	void mouseMove(int windowId, int x, int y, int movementX, int movementY) {
		// @@TODO: Figure out how to capture the mouse outside the window
		if (rotate) {
			cameraRotX += (float)((mousePressY - y) * CAMERA_ROTATION_SPEED_X);
			cameraRotY -= (float)((mousePressX - x) * CAMERA_ROTATION_SPEED_Y);
			mousePressX = x;
			mousePressY = y;
		}
	}

	void mousePress(int windowId, int button, int x, int y) {
		rotate = true;
		mousePressX = x;
		mousePressY = y;
	}

	void mouseRelease(int windowId, int button, int x, int y) {
		rotate = false;
	}
}

int kore(int argc, char** argv) {
	Kore::System::setName("TUD Game Technology - ");
	Kore::System::setup();
	Kore::WindowOptions options;
	options.title = "Solution 3";
	options.width = width;
	options.height = height;
	options.x = 100;
	options.y = 100;
	options.targetDisplay = -1;
	options.mode = WindowModeWindow;
	options.rendererOptions.depthBufferBits = 16;
	options.rendererOptions.stencilBufferBits = 8;
	options.rendererOptions.textureFormat = 0;
	options.rendererOptions.antialiasing = 0;
	Kore::System::initWindow(options);

	initGraphics();
	Kore::System::setCallback(update);

	Kore::Mixer::init();
	Kore::Audio::init();


	startTime = System::time();

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
