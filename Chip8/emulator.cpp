#include "emulator.h"

#define NO_KEY 255

void initState(State *state)
{
	srand(time(NULL));
	memset(state->memory, 0, MAX_MEMORY);
	memset(state->V, 0, 16);
	state->stackLevel = 0;
	clearDisplay(state->display);
	loadSprites(state);
	state->PC = 0x200; // Starts at 512
	memset(state->modifiedAddresses, 0xFFFFFFFF, 16 * sizeof(uint32));
	state->modifiedAddressesCount = 0;
}

byte sprites[16 * 5] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0,
	0x20, 0x60, 0x20, 0x20, 0x70,
	0xF0, 0x10, 0xF0, 0x80, 0xF0,
	0xF0, 0x10, 0xF0, 0x10, 0xF0,
	0x90, 0x90, 0xF0, 0x10, 0x10,
	0xF0, 0x80, 0xF0, 0x10, 0xF0,
	0xF0, 0x80, 0xF0, 0x90, 0xF0,
	0xF0, 0x10, 0x20, 0x40, 0x40,
	0xF0, 0x90, 0xF0, 0x90, 0xF0,
	0xF0, 0x90, 0xF0, 0x10, 0xF0,
	0xF0, 0x90, 0xF0, 0x90, 0x90,
	0xE0, 0x90, 0xE0, 0x90, 0xE0,
	0xF0, 0x80, 0x80, 0x80, 0xF0,
	0xE0, 0x90, 0x90, 0x90, 0xE0,
	0xF0, 0x80, 0xF0, 0x80, 0xF0,
	0xF0, 0x80, 0xF0, 0x80, 0x80
};

void loadSprites(State *state)
{
	memcpy(state->memory, sprites, 16 * 5);
}

uint32 doStep(State *state)
{
	uint32 op;
	readInstruction(&op, state->memory + state->PC);
	state->PC += 2; // Should increment this before or after?

	memset(state->modifiedAddresses, 0xFFFFFFFF, 16 * sizeof(uint32));
	state->modifiedAddressesCount = 0;

	uint32 opType;
	getOpType(&opType, op);
	switch (opType)
	{
	case 0x0000:
		processOpType0(op, state);
		break;
	case 0x1000:
		processOpType1(op, state);
		break;
	case 0x2000:
		processOpType2(op, state);
		break;
	case 0x3000:
		processOpType3(op, state);
		break;
	case 0x4000:
		processOpType4(op, state);
		break;
	case 0x5000:
		processOpType5(op, state);
		break;
	case 0x6000:
		processOpType6(op, state);
		break;
	case 0x7000:
		processOpType7(op, state);
		break;
	case 0x8000:
		processOpType8(op, state);
		break;
	case 0x9000:
		processOpType9(op, state);
		break;
	case 0xA000:
		processOpTypeA(op, state);
		break;
	case 0xB000:
		processOpTypeB(op, state);
		break;
	case 0xC000:
		processOpTypeC(op, state);
		break;
	case 0xD000:
		processOpTypeD(op, state);
		break;
	case 0xE000:
		processOpTypeE(op, state);
		break;
	case 0xF000:
		processOpTypeF(op, state);
		break;
	default:
		printf("Unsupported instruction type: %#010x\n", opType);
		break;
	}

	return op;
}

void updateTimers(State *state)
{
	if (state->delayTimer > 0)
	{
		--state->delayTimer;
	}
	if (state->soundTimer > 0)
	{
		--state->soundTimer;
	}
}

void writeOpToMemory(void* dst, uint32 op)
{
	uint32 *address = (uint32*)dst;
	*address |= op;
}

void readInstruction(uint32 *dst, byte *src)
{
	byte b1 = *src;
	byte b2 = *(src + 1);
	*dst = b1;
	*dst <<= 8;
	*dst |= b2;
}

void getOpType(uint32 *dst, uint32 op)
{
	*dst = op;
	*dst &= 0x0000F000;
}

void loadProgramFromFile(const char *filePath, State *state)
{
	FILE *file;
	int error = fopen_s(&file, filePath, "rb");
	assert(error == 0);
	// Obtain file size
	long size;
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	// Read the file
	byte *buffer = (byte*)malloc(sizeof(byte)*size);
	int result = fread(buffer, 1, size, file);
	assert(result == size);

	memcpy(state->memory + 0x200, buffer, size);
	fclose(file);
	free(buffer);
}

void printDisplay(byte *display)
{
	system("cls");
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
	switch (op)
	{
	case 0x00E0: // Clears the screen.
		clearDisplay(state->display);
		break;
	case 0x00EE: // Returns from a subroutine.
		--state->stackLevel;
		state->PC = state->stack[state->stackLevel];
		break;
	case PROGRAM_END:
		break;
	default:
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
	state->stack[state->stackLevel] = state->PC;
	uint32 value = op & 0x0FFF;
	state->PC = value;
	++state->stackLevel;
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

void writeToMemory(State *state, uint32 address, byte value)
{
	state->memory[address] = value;
	int count = state->modifiedAddressesCount;
	if (count < 16)
	{
		state->modifiedAddresses[count] = address;
		state->modifiedAddressesCount++;
	}
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
		state->V[0xF] = temp >= state->V[X];
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
	// 0x9XY0 - Skips the next instruction if VX doesn't equal VY.
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
	uint32 address = op & 0x0FFF;
	state->I = address;
}

void processOpTypeB(uint32 op, State *state)
{
	// 0xBNNN - Jumps to the address NNN plus V0.
	uint32 address = op & 0x0FFF;
	state->PC = address + state->V[0];
}

void processOpTypeC(uint32 op, State *state)
{
	// 0xCXNN - Sets VX to the result of a bitwise and operation on a random number and NN.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 value = op & 0x00FF;
	uint32 r = rand();
	r &= 0x00FF; // Just in case
	state->V[X] = value & r;
}

void processOpTypeD(uint32 op, State *state)
{
	// DXYN -	Sprites stored in memory at location in index register (I), 
	//			8bits wide. Wraps around the screen. If when drawn, clears a pixel, 
	//			register VF is set to 1 otherwise it is zero. All drawing is XOR 
	//			drawing (i.e. it toggles the screen pixels). Sprites are drawn starting 
	//			at position VX, VY. N is the number of 8bit rows that need to be drawn. 
	//			If N is greater than 1, second line continues at position VX, VY+1, and so on.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 Y = op & 0x00F0;
	Y = Y >> 4;
	byte *address = state->memory + state->I;
	uint32 rows = op & 0x000F;
	byte initialPosX = state->V[X];
	byte posY = state->V[Y];
	byte changed = 0;
	for (uint32 i = 0; i < rows; ++i)
	{
		byte toDraw = *address; // Here we have the byte to draw in a row
		byte posX = initialPosX;
		for (int p = 7; p >= 0; --p)
		{
			if (posX > 64)
			{
				posX = 0;
			}
			uint32 index = posY*DISPLAY_W + posX;
			byte pixel = toDraw >> p;
			pixel &= 0x01;
			changed |= pixel & state->display[index];
			state->display[index] ^= pixel;
			++posX;
		}
		++posY;
		++address;
	}
	state->V[0xF] = changed;
}

void processOpTypeE(uint32 op, State *state)
{
	// 0xE??? - Keyboard skips
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 subtype = op & 0x00FF;
	byte key = state->keyboard[state->V[X]];
	switch (subtype)
	{
	case 0x9E: // Skips the next instruction if the key stored in VX is pressed.
		state->PC += 2 * key;
		break;
	case 0xA1: // Skips the next instruction if the key stored in VX isn't pressed.
		state->PC += 2 * !key;
		break;
	default:
		printf("Instruction of type E not supported: %#010x\n", op);
		assert(false);
	}
}

byte getFirstKeyPressed(State *state)
{
	for (int i = 0; i < 15; ++i)
	{
		if (state->keyboard[i])
		{
			return i;
		}
	}

	return NO_KEY;
}

void processOpTypeF(uint32 op, State *state)
{
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 subtype = op & 0x00FF;
	byte key = state->keyboard[state->V[X]];
	byte temp;
	byte digit;
	switch (subtype)
	{
	case 0x07: // Sets VX to the value of the delay timer.
		state->V[X] = state->delayTimer;
		break;
	case 0x0A: // A key press is awaited, and then stored in VX.
		temp = getFirstKeyPressed(state);
		if (temp == NO_KEY)
		{
			state->PC -= 2; // Waiting for keyboard, let's repeat the same instruction again
		}
		else
		{
			state->V[X] = temp;
		}
		break;
	case 0x15: // Sets the delay timer to VX.
		state->delayTimer = state->V[X];
		break;
	case 0x18: // Sets the sound timer to VX.
		state->soundTimer = state->V[X];
		break;
	case 0x1E: // Adds VX to I
		temp = state->V[X];
		state->V[X] = temp + state->I;
		state->V[0xF] = temp > state->V[X];
		break;
	case 0x29: // Sets I to the location of the sprite for the character in VX. 
		// Characters 0-F (in hexadecimal) are represented by a 4x5 font.
		state->I = state->V[X] * 0x5;
		break;
	case 0x33: // Stores the Binary-coded decimal representation of VX, with the most 
		// significant of three digits at the address in I, the middle digit 
		// at I plus 1, and the least significant digit at I plus 2. 
		// (In other words, take the decimal representation of VX, place the 
		// hundreds digit in memory at location in I, the tens digit at location I+1, 
		// and the ones digit at location I+2.)
		//state->I = state->V[X] * 0x5;
		temp = state->V[X];
		digit = temp / 100;
		writeToMemory(state, state->I, digit);
		digit = temp % 100;
		writeToMemory(state, state->I + 1, digit / 10);
		digit = temp % 10;
		writeToMemory(state, state->I + 2, digit);
		break;
	case 0x55: // Stores V0 to VX in memory starting at address I.
		for (uint32 i = 0; i <= X; i++)
		{
			writeToMemory(state, state->I + i, state->V[i]);
		}
		break;
	case 0x65: // Fills V0 to VX with values from memory starting at address I.
		for (uint32 i = 0; i <= X; i++)
		{
			state->V[i] = state->memory[state->I + i];
		}
		break;
	default:
		printf("Instruction of type F not supported: %#010x\n", op);
		assert(false);
	}
}
