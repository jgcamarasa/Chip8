#include "GLFW\glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <Windows.h>
#include "emulator.h"
#include "gui.h"

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

void updateStateKeyboard(State *state, GLFWwindow *window)
{
	state->keyboard[0] = glfwGetKey(window, GLFW_KEY_1);
	state->keyboard[1] = glfwGetKey(window, GLFW_KEY_2);
	state->keyboard[2] = glfwGetKey(window, GLFW_KEY_3);
	state->keyboard[3] = glfwGetKey(window, GLFW_KEY_4);
	state->keyboard[4] = glfwGetKey(window, GLFW_KEY_Q);
	state->keyboard[5] = glfwGetKey(window, GLFW_KEY_W);
	state->keyboard[6] = glfwGetKey(window, GLFW_KEY_E);
	state->keyboard[7] = glfwGetKey(window, GLFW_KEY_R);
	state->keyboard[8] = glfwGetKey(window, GLFW_KEY_A);
	state->keyboard[9] = glfwGetKey(window, GLFW_KEY_S);
	state->keyboard[10] = glfwGetKey(window, GLFW_KEY_D);
	state->keyboard[11] = glfwGetKey(window, GLFW_KEY_F);
	state->keyboard[12] = glfwGetKey(window, GLFW_KEY_Z);
	state->keyboard[13] = glfwGetKey(window, GLFW_KEY_X);
	state->keyboard[14] = glfwGetKey(window, GLFW_KEY_C);
	state->keyboard[15] = glfwGetKey(window, GLFW_KEY_V);
}

int main()
{
	State state;
	Controller controller;
	controller.status = CONTROLLER_STATUS_PAUSED;
	controller.state = &state;
	
	initState(&state);
	loadProgramFromFile("pong.ch8", &state);

	// Window creation
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIN_W, WIN_H, "Castor-8", NULL, NULL);
	assert(window != NULL);
	glfwMakeContextCurrent(window);

	// ImGui
	ImGui_ImplGlfw_Init(window, true);

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

		glfwPollEvents();

#define TIMER_PERIOD_MILLI (1 / 60) * 1000
		if (stateTimer > TIMER_PERIOD_MILLI)
		{
			updateTimers(&state);
			stateTimer -= TIMER_PERIOD_MILLI;
		}
		updateStateKeyboard(&state, window);

		uint32 op;
		if (controller.status == CONTROLLER_STATUS_RUNNING)
		{
			op = doStep(&state);
		}

		drawGUI(&state, &controller);

		// Rendering
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui::Render();
		renderState(&state);
		glfwSwapBuffers(window);

		// stop timer
		QueryPerformanceCounter(&t2);
		elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;

		stateTimer += elapsedTime;
	}

	ImGui_ImplGlfw_Shutdown();
	glfwTerminate();
	return 0;
}