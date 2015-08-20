#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned int uint32;
typedef unsigned char byte;

#define MAX_MEMORY 4096
#define DISPLAY_W 64
#define DISPLAY_H 32

#define OP_CLS 0x00E0
#define OP_RET 0x00EE
#define OP_SETV(val)
#define PROGRAM_END 0xFFFF

uint32 program[] = {
	OP_CLS,
	OP_RET,
	PROGRAM_END
};


void writeOpToMemory(void* dst, unsigned int op)
{
	uint32 *address = (uint32*)dst;
	op << 4;
	*address |= op;
}

void readInstruction(uint32 *dst, void *src)
{
	uint32 *mem = (uint32*)src;
	*dst = *mem;
	*dst &= 0x0000FFFF;
}

void loadDefaultProgram(byte* dst)
{
	uint32 count = 0;
	while (program[count] != PROGRAM_END)
	{
		writeOpToMemory(dst + count*2, program[count]);
		++count;
	} 
}

void renderDisplay(byte *display)
{
	for (uint32 y = 0; y < DISPLAY_H; ++y)
	{
		for (uint32 x = 0; x < DISPLAY_W; ++x)
		{
			printf("%d", display[y*DISPLAY_W + x]);
		}
		printf("\n");
	}
	printf("\n");
}

void clearDisplay(byte *display)
{
	memset(display, 0, DISPLAY_W*DISPLAY_H);
}

int main()
{
	byte memory[MAX_MEMORY];
	byte V[16];				// V0-VF, Registers (8 bits). VF = Carry Flag
	byte I[2];				// Address register (16 bits)
	byte stack[64];			// Stack
	byte delayTimer;		// Delay timer TODO: Check if it's a byte or what
	byte soundTimer;		// Sound timer
	byte keyboard[16];		// Hex Keyboard. Keys from 0 to F
	byte display[DISPLAY_W*DISPLAY_H];	// Byte used as a bool for each pixel
	uint32 PC;				// Program counter
	
	// Each opcode has 4 bytes
	
	// Initialization
	memset(memory, 0, MAX_MEMORY);
	PC = 0x200; // Starts at 512
	loadDefaultProgram(memory+PC);

	int exit = 0;
	while (!exit)
	{
		uint32 op;
		readInstruction(&op, memory + PC);
		PC += 2; // Should increment this before or after?
		switch (op)
		{
		case OP_CLS:
			clearDisplay(display);
			break;
		default:
			printf("Unrecognized instruction: %#010x\n", op);
			break;
		}

		renderDisplay(display);

		exit = PC >= 0x204;
	}

	system("pause");
	return 0;
}