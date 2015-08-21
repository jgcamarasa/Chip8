#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned int uint32;
typedef unsigned char byte;

#define MAX_MEMORY 4096
#define DISPLAY_W 64
#define DISPLAY_H 32

#define OP_CLS 0x00E0
#define OP_RET 0x00EE
#define OP_JMP(NNN) (0x1000 | NNN)
#define OP_CALL(NNN) (0x2000 | NNN)
#define OP_SKIP_EQ(X, NN) ((0x3000 | NN) | (X << 8))
#define OP_SKIP_NEQ(X, NN) ((0x4000 | NN) | (X << 8))
#define OP_SKIP_EQ2(X, Y) ((0x5000 | X << 8) | (Y << 4))
#define OP_SETV(X, NN) ((0x6000 | NN) | (X << 8))
#define OP_ADDV(X, NN) ((0x7000 | NN) | (X << 8))
#define OP_ARI(X, Y, op) ((0x8000 | X << 8) | (Y << 4) | (op))
#define PROGRAM_END 0x0000

void testOpWrite()
{
	if (OP_JMP(0x234) != 0x1234) 
	{
		printf("OP_JMP test failed\n"); 
		assert(false);
	}
	if (OP_CALL(0x234) != 0x2234)
	{
		printf("OP_CALL test failed\n");
		assert(false);
	}
	if (OP_SKIP_EQ(2, 0x34) != 0x3234)
	{
		uint32 val1 = OP_SKIP_EQ(2, 0x34);
		uint32 val2 = 0x3234;
		bool comp = val1 == val2;
		printf("OP_SKIP_EQ test failed: \n");
		assert(false);
	}
	if (OP_SKIP_NEQ(2, 0x34) != 0x4234)
	{
		printf("OP_SKIP_NEQ test failed\n");
		assert(false);
	}
	if (OP_SKIP_EQ2(2, 3) != 0x5230)
	{
		printf("OP_SKIP_EQ2 test failed\n");
		assert(false);
	}
	if (OP_SETV(2, 0x34) != 0x6234)
	{
		printf("OP_SETV test failed\n");
		assert(false);
	}
	if (OP_ADDV(2, 0x34) != 0x7234)
	{
		printf("OP_ADDV test failed\n");
		assert(false);
	}
	if (OP_ARI(0xA, 0xB, 3) != 0x8AB3)
	{
		printf("OP_ARI test failed\n");
		assert(false);
	}
}

uint32 program[] = {
	OP_CLS,
	OP_SETV(0, 0xFF),
	OP_SETV(1, 0x01),
	OP_ARI(0, 1, 4),
	OP_SKIP_EQ(0, 0x00),
	OP_JMP(0x200),
	PROGRAM_END
};

struct State
{
	byte memory[MAX_MEMORY];
	byte V[16];				// V0-VF, Registers (8 bits). VF = Carry Flag
	byte stack[64];			// Stack
	byte I[2];				// Address register (16 bits)
	byte delayTimer;		// Delay timer TODO: Check if it's a byte or what
	byte soundTimer;		// Sound timer
	byte keyboard[16];		// Hex Keyboard. Keys from 0 to F
	byte display[DISPLAY_W*DISPLAY_H];	// Byte used as a bool for each pixel
	uint32 PC;				// Program counter
};

void writeOpToMemory(void* dst, uint32 op)
{
	uint32 *address = (uint32*)dst;
	*address |= op;
}

void readInstruction(uint32 *dst, void *src)
{
	uint32 *mem = (uint32*)src;
	*dst = *mem;
	*dst &= 0x0000FFFF;
}

void getOpType(uint32 *dst, uint32 op)
{
	*dst = op;
	*dst &= 0x0000F000;
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

void processOpType0(uint32 op, State *state)
{
	if (op == OP_CLS)
	{
		clearDisplay(state->display);
	}
	else if (op != PROGRAM_END)
	{
		printf("Instruction of type 0 not supported: %#010x\n", op);
		assert(false);
	}
}

void processOpType1(uint32 op, State *state)
{
	// 0x1NNN - Jumps to address NNN.
	uint32 address = op & 0x0000FFF;
	state->PC = address;
}

void processOpType2(uint32 op, State *state)
{
	// 0x2NNN - Calls subroutine at NNN.
	printf("Instruction of type 2 not supported: %#010x\n", op);
	assert(false);
}

void processOpType3(uint32 op, State *state)
{
	// 0x3XNN - Skips the next instruction if VX equals NN.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 value = op & 0x00FF;
	uint32 comp = value == state->V[X];
	state->PC += 2 * comp;
}

void processOpType4(uint32 op, State *state)
{
	// 0x4XNN - Skips the next instruction if VX doesn't equal NN.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 value = op & 0x00FF;
	uint32 comp = value != state->V[X];
	state->PC += 2 * comp;
}

void processOpType5(uint32 op, State *state)
{
	// 0x5XY0 - Skips the next instruction if VX equals VY.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 Y = op & 0x00F0;
	Y = Y >> 4;
	uint32 comp = state->V[X] == state->V[Y];
	state->PC += 2 * comp;
}

void processOpType6(uint32 op, State *state)
{
	// 0x6XNN - Sets VX to NN.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 value = op & 0x00FF;
	state->V[X] = value;
}

void processOpType7(uint32 op, State *state)
{
	// 0x7XNN - Adds NN to VX.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 value = op & 0x00FF;
	state->V[X] += value;
}

void processOpType8(uint32 op, State *state)
{
	// 0x8XY? - Arithmetic
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 Y = op & 0x00F0;
	Y = Y >> 4;

	byte temp;
	uint32 subtype = op & 0x000F;
	switch (subtype)
	{
	case 0x0: // Sets VX to the value of VY
		state->V[X] = state->V[Y];
		break;
	case 0x1: // Sets VX to VX or VY. (bitwise)
		state->V[X] |= state->V[Y];
		break;
	case 0x2: // Sets VX to VX and VY. (bitwise)
		state->V[X] &= state->V[Y];
		break;
	case 0x3: // Sets VX to VX xor VY. (bitwise)
		state->V[X] ^= state->V[Y];
		break;
	case 0x4: // Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
		temp = state->V[X];
		state->V[X] += state->V[Y];
		state->V[0xF] = temp > state->V[X];
		break;
	case 0x5: // VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		temp = state->V[X];
		state->V[X] -= state->V[Y];
		state->V[0xF] = temp < state->V[X];
		break;
	case 0x6: // Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
		state->V[0xF] = state->V[X] & 0x0001;
		state->V[X] = state->V[X] >> 1;
		break;
	case 0x7: // Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		temp = state->V[X];
		state->V[X] = state->V[Y] - state->V[X];
		state->V[0xF] = temp >= state->V[X];
		break;
	case 0xE: // Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
		state->V[0xF] = state->V[X] & 0x00080;
		state->V[0xF] >>= 7;
		state->V[X] = state->V[X] << 1;
		break;
	default:
		printf("Instruction of type 8 not supported: %#010x\n", op);
		assert(false);
		break;
	}
	
}

void processOpType9(uint32 op, State *state)
{
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 Y = op & 0x00F0;
	Y = Y >> 4;
	uint32 comp = state->V[X] != state->V[Y];
	state->PC += 2 * comp;
}

void processOpTypeA(uint32 op, State *state)
{
	// 0xANNN - Sets I to the address NNN.
	printf("Instruction of type A not supported: %#010x\n", op);
	assert(false);
}

void processOpTypeB(uint32 op, State *state)
{
	// 0xBNNN - Jumps to the address NNN plus V0.
	printf("Instruction of type B not supported: %#010x\n", op);
	assert(false);
}

void processOpTypeC(uint32 op, State *state)
{
	// 0xCXNN - Sets VX to the result of a bitwise and operation on a random number and NN.
	printf("Instruction of type C not supported: %#010x\n", op);
	assert(false);
}

void processOpTypeD(uint32 op, State *state)
{
	// DXYN -	Sprites stored in memory at location in index register (I), 
	//			8bits wide. Wraps around the screen. If when drawn, clears a pixel, 
	//			register VF is set to 1 otherwise it is zero. All drawing is XOR 
	//			drawing (i.e. it toggles the screen pixels). Sprites are drawn starting 
	//			at position VX, VY. N is the number of 8bit rows that need to be drawn. 
	//			If N is greater than 1, second line continues at position VX, VY+1, and so on.
	printf("Instruction of type D not supported: %#010x\n", op);
	assert(false);
}

void processOpTypeE(uint32 op, State *state)
{
	// 0xE??? - Skips
	printf("Instruction of type E not supported: %#010x\n", op);
	assert(false);
}

void processOpTypeF(uint32 op, State *state)
{
	// 0xF??? - 
	printf("Instruction of type F not supported: %#010x\n", op);
	assert(false);
}


int main()
{
	testOpWrite();
	State state;
	
	// Initialization
	memset(state.memory, 0, MAX_MEMORY);
	memset(state.V, 0, 16);
	state.PC = 0x200; // Starts at 512
	loadDefaultProgram(state.memory + state.PC);

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

		renderDisplay(state.display);

		exit = (op == PROGRAM_END);
	}

	system("pause");
	return 0;
}