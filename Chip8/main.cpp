#include "GLFW\glfw3.h"
#include <Windows.h>
#include "emulator.h"

#define DISPLAY_FACTOR 8

void renderState(State *state)
{
	const int outputW = DISPLAY_W * DISPLAY_FACTOR;
	const int outputH = DISPLAY_H * DISPLAY_FACTOR;
	const int inputSize = DISPLAY_W * DISPLAY_H;
	const int outputSize = outputW * outputH;
	byte data[outputSize];
	for (int y = 0; y < outputH; ++y)
	{
		for (int x = 0; x < outputW; ++x)
		{
			int srcX = x / DISPLAY_FACTOR;
			int srcY = DISPLAY_H - y / DISPLAY_FACTOR -1;
			data[x + y * outputW] = state->display[srcX + srcY * DISPLAY_W] * 255;
		}
	}
	glDrawPixels(DISPLAY_W * DISPLAY_FACTOR, DISPLAY_H * DISPLAY_FACTOR, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
}

byte keyboard[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };



int main()
{
	State state;
	
	initState(&state);
	loadProgramFromFile("rndtest.ch8", &state);

	// Window creation
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(DISPLAY_W * DISPLAY_FACTOR, DISPLAY_H * DISPLAY_FACTOR, "Castor-8", NULL, NULL);
	assert(window != NULL);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	// State timer
	double stateTimer = 0.0f;

	int exit = 0;
	while (!exit && !glfwWindowShouldClose(window))
	{
		LARGE_INTEGER frequency;        // ticks per second
		LARGE_INTEGER t1, t2;           // ticks
		double elapsedTime;

		// get ticks per second
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&t1);

#define TIMER_PERIOD_MILLI (1 / 60) * 1000
		if (elapsedTime > TIMER_PERIOD_MILLI)
		{
			updateTimers(&state);
			elapsedTime -= TIMER_PERIOD_MILLI;
		}
		uint32 op = doStep(&state);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderState(&state);
		glfwSwapBuffers(window);

		glfwPollEvents();

		// stop timer
		QueryPerformanceCounter(&t2);
		elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;

		stateTimer += elapsedTime;

		exit = (op == PROGRAM_END);
	}
	return 0;
}