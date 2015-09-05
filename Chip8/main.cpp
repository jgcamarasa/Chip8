#include "GLFW\glfw3.h"

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

	int exit = 0;
	while (!exit && !glfwWindowShouldClose(window))
	{
		uint32 op = doStep(&state);
		renderState(&state);

		glfwSwapBuffers(window);
		glfwPollEvents();

		exit = (op == PROGRAM_END);
	}
	return 0;
}