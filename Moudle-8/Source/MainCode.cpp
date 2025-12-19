///////////////////////////////////////////////////////////////////////////////
// 2D Game Program
// ===============
// This program implements a simple 2D game using OpenGL and GLFW. The game
// involves circular objects (referred to as "balls") that move within a window 
// and interact with static rectangular bricks. The primary features of the game
// include:
// 
// - **Bricks**: Represented as either reflective or destructible. Reflective 
//   bricks change the direction of the balls upon collision, while destructible
//   bricks disappear when hit.
// - **Balls**: Circular objects that move randomly within the window. Their 
//   motion is influenced by collisions with bricks and window boundaries.
// - **Input Handling**: Players can spawn new balls by pressing the spacebar or 
//   close the game window by pressing the ESC key.
//
// Key Features:
// -------------
// - **Brick Types**:
//   - Reflective bricks alter the direction of the balls.
//   - Destructible bricks disappear upon collision.
// - **Ball Motion**:
//   - Balls move in random directions and bounce upon hitting boundaries.
//   - Collisions with bricks are handled dynamically.
// - **User Input**:
//   - Spacebar spawns a new ball at the center with random colors.
//   - ESC closes the application.
// 
// Dependencies:
// -------------
// - GLFW for window creation and input handling.
// - OpenGL for rendering graphics.
// 
// Author: Mike Brown
// Date: 12/19/2025
// Assignment: 8-2 Assignment - CS-330 Computational Graphics and Visualization
///////////////////////////////////////////////////////////////////////////////

#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

bool gBallStarted = false;


// ---------------------------------------------------------------------------
// Performance / control limits
// ---------------------------------------------------------------------------
const int   MAX_BALLS = 3;      // hard cap to limit total active balls
const int   CIRCLE_SEGMENTS = 36; 
bool gSpaceWasDown = false;       // spawn on tap (not while held)
int  ballsUsed = 0;
bool maxBallsUsed = false;

class Brick
{
public:
	float red, green, blue;
	float x, y, width;
	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
		onoff = ON;
	};

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.03;
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left
	bool active = true;     // used to safely remove circles after collisions
	int  splitCooldown = 0; // frames until circle can split again

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				direction = GetRandomDirection();
				x = x + 0.03;
				y = y + 0.04;
			}
		}
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				brk->onoff = OFF;
			}
		}
	}

	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	void MoveOneStep()
	{
		if (splitCooldown > 0) splitCooldown--;

		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y > -1 + radius)
			{
				y -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius)
			{
				x += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius) {
				y += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}
	}

	void DrawCircle()
	{
		if (!active) return;

		glColor3f(red, green, blue);

		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(x, y); // center
		for (int i = 0; i <= CIRCLE_SEGMENTS; i++)
		{
			float degInRad = (360.0f / CIRCLE_SEGMENTS) * i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}


};


vector<Circle> world;
void HandleCircleCircleCollisions();


int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "8-2 Assignment", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// ---------------------------------------------------------------------------
	// Brick wall
	// ---------------------------------------------------------------------------
	vector<Brick> bricks;

	int rows = 6;
	int cols = 10;

	float brickW = 0.16f;  // square width
	float gapX = 0.02f;
	float gapY = 0.02f;

	// Start near the top (inverted Y system: -1 is top)
	float startX = -0.85f;
	float startY = -0.80f;

	// Alternate brick types by row for interest
	BRICKTYPE typeToUse = (rows < 2) ? REFLECTIVE : DESTRUCTABLE;


	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			float x = startX + c * (brickW + gapX);
			float y = startY + r * (brickW + gapY);

			// Color variation by row
			float rr = 0.20f + 0.12f * r;
			float gg = 0.85f - 0.10f * r;
			float bb = 0.95f - 0.12f * r;

			bricks.push_back(Brick(typeToUse, x, y, brickW, rr, gg, bb));
		}
	}

;


	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		// -----------------------------
		// Draw bricks
		// -----------------------------
		for (int b = 0; b < bricks.size(); b++)
		{
			bricks[b].drawBrick();
		}

		// -----------------------------
		// Update + draw circles
		// -----------------------------
		for (int i = 0; i < world.size(); i++)
		{
			if (!world[i].active) continue;

			// collide with all bricks
			for (int b = 0; b < bricks.size(); b++)
			{
				world[i].CheckCollision(&bricks[b]);
			}

			// only move after Space starts the game
			if (gBallStarted)
			{
				world[i].MoveOneStep();
			}

			world[i].DrawCircle();
		}

		// circle-circle split AFTER movement
		HandleCircleCircleCollisions();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}



	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


void processInput(GLFWwindow* window)
{
	// Space starts movement
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		gBallStarted = true;
	}

	// Tap Space to spawn ONE new ball (not while held)
	bool spaceDown = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

	if (spaceDown && !gSpaceWasDown)
	{
		if (!maxBallsUsed) // correct boolean check
		{
			if ((int)world.size() < MAX_BALLS)
			{

				double r = (double)rand() / (double)RAND_MAX;
				double g = (double)rand() / (double)RAND_MAX;
				double b = (double)rand() / (double)RAND_MAX;

				// spawn near top
				Circle B(0.0, -0.95f, 02, 3, 0.05f, (float)r, (float)g, (float)b);
				world.push_back(B);
				ballsUsed++;
				if (ballsUsed >= MAX_BALLS)
				{
					maxBallsUsed = true;
					cout << "Max balls reached!" << endl;
				}
			}
			else
			{
				maxBallsUsed = true;
				cout << "Max balls reached!" << endl;
			}
		}
		else
		{
			cout << "Max balls reached!" << endl;
		}
	}


	gSpaceWasDown = spaceDown;

	// ESC closes
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}


// ---------------------------------------------------------------------------
// Circle-Circle collision: controlled split (no runaway)
// ---------------------------------------------------------------------------
void HandleCircleCircleCollisions()
{
	// Spawn buffer to avoid modifying world while iterating pairs
	vector<Circle> spawn;

	for (int i = 0; i < world.size(); i++)
	{
		if (!world[i].active || world[i].splitCooldown > 0) continue;

		for (int j = i + 1; j < world.size(); j++)
		{
			if (!world[j].active || world[j].splitCooldown > 0) continue;

			float dx = world[i].x - world[j].x;
			float dy = world[i].y - world[j].y;
			float rr = world[i].radius + world[j].radius;

			if ((dx * dx + dy * dy) <= (rr * rr))
			{
				// midpoint of collision
				float cx = (world[i].x + world[j].x) * 0.5f;
				float cy = (world[i].y + world[j].y) * 0.5f;

				// deactivate originals
				world[i].active = false;
				world[j].active = false;

				// spawn only 2 child circles per collision
				int children = 2;
				float childRad = world[i].radius * 0.65f;
				if (childRad < 0.02f) childRad = 0.02f;

				for (int k = 0; k < children; k++)
				{
					if ((int)(world.size() + spawn.size()) >= MAX_BALLS) break;

					double r = (double)rand() / (double)RAND_MAX;
					double g = (double)rand() / (double)RAND_MAX;
					double b = (double)rand() / (double)RAND_MAX;

					Circle c(cx, cy, 2, 2, childRad, r, g, b);
					c.direction = c.GetRandomDirection();
					c.speed = 0.02f;        // keep child speed reasonable
					c.splitCooldown = 25;   // prevents chain-splitting
					spawn.push_back(c);
				}

				// once i splits, stop looking for more collisions for it
				break;
			}
		}
	}

	// Compact active circles
	vector<Circle> keep;
	keep.reserve(MAX_BALLS);

	for (int i = 0; i < world.size(); i++)
	{
		if (world[i].active)
		{
			keep.push_back(world[i]);
			if ((int)keep.size() >= MAX_BALLS) break;
		}
	}

	for (int i = 0; i < spawn.size(); i++)
	{
		if ((int)keep.size() >= MAX_BALLS) break;
		keep.push_back(spawn[i]);
	}

	world = keep;
}
