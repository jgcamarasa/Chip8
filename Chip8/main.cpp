#include "GLFW\glfw3.h"

#include "emulator.h"

#define DISPLAY_FACTOR 2

void renderState(State *state)
{
	byte data[DISPLAY_W * DISPLAY_H * DISPLAY_FACTOR * DISPLAY_FACTOR];
	for (uint32 y = 0; y < DISPLAY_H; ++y)
	{
		for (uint32 x = 0; x < DISPLAY_W; ++x)
		{
			uint32 srcPos = y*DISPLAY_W + x;
			byte value = state->display[srcPos] * 255;
			for (int i = 0; i < DISPLAY_FACTOR; ++i)
			{
				for (int j = 0; j < DISPLAY_FACTOR; ++j)
				{
					uint32 destPos = srcPos * DISPLAY_FACTOR + i + DISPLAY_W * DISPLAY_FACTOR * j;
					data[destPos] = value;
				}
			}

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
	while (!exit)
	{
		uint32 op;
		readInstruction(&op, state.memory + state.PC);
		state.PC += 2; // Should increment this before or after?
		
		uint32 opType;
		getOpType(&opType, op);
		switch (opType)
		{
		case 0x0000:
			processOpType0(op, &state);
			break;
		case 0x1000:
			processOpType1(op, &state);
			break;
		case 0x2000:
			processOpType2(op, &state);
			break;
		case 0x3000:
			processOpType3(op, &state);
			break;
		case 0x4000:
			processOpType4(op, &state);
			break;
		case 0x5000:
			processOpType5(op, &state);
			break;
		case 0x6000:
			processOpType6(op, &state);
			break;
		case 0x7000:
			processOpType7(op, &state);
			break;
		case 0x8000:
			processOpType8(op, &state);
			break;
		case 0x9000:
			processOpType9(op, &state);
			break;
		case 0xA000:
			processOpTypeA(op, &state);
			break;
		case 0xB000:
			processOpTypeB(op, &state);
			break;
		case 0xC00:
			processOpTypeC(op, &state);
			break;
		case 0xD000:
			processOpTypeD(op, &state);
			break;
		case 0xE000:
			processOpTypeE(op, &state);
			break;
		case 0xF000:
			processOpTypeF(op, &state);
			break;
		default:
			printf("Unsupported instruction type: %#010x\n", opType);
			break;
		}

		//printDisplay(state.display);
		renderState(&state);
		glfwSwapBuffers(window);
		glfwPollEvents();

		exit = (op == PROGRAM_END);
	}

	system("pause");
	return 0;
}