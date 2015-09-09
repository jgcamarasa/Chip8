#pragma once 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

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
#define OP_SKIP_NEQ2(X, Y) ((0x9000 | X << 8) | (Y << 4))
#define OP_SETI(NNN) (0xA000 | NNN)
#define OP_JMP2(NNN) (0xB000 | NNN)
#define OP_RND(X, NN) ((0xC000 | NN) | (X << 8))
#define OP_DRW(X, Y, N) ((0xD000 | X << 8) | (Y << 4) | (N))
#define PROGRAM_END 0x0000

struct State
{
	byte memory[MAX_MEMORY];
	byte V[16];				// V0-VF, Registers (8 bits). VF = Carry Flag
	uint32 stack[64];			// Stack
	uint32 stackLevel;		// Current stack level
	uint32 I;				// Address register (16 bits)
	byte delayTimer;		// Delay timer TODO: Check if it's a byte or what
	byte soundTimer;		// Sound timer
	byte keyboard[16];		// Hex Keyboard. Keys from 0 to F
	byte display[DISPLAY_W*DISPLAY_H];	// Byte used as a bool for each pixel
	uint32 PC;				// Program counter

	uint32 modifiedAddresses[16]; // For debugging
	uint32 modifiedAddressesCount;
};

void initState(State *state);

void loadSprites(State *state);

uint32 doStep(State *state); // returns current instruction

void updateTimers(State *state);

void writeOpToMemory(void* dst, uint32 op);

void readInstruction(uint32 *dst, byte *src);

void getOpType(uint32 *dst, uint32 op);

void loadProgramFromFile(const char *filePath, State *state);

void printDisplay(byte *display);

void clearDisplay(byte *display);

void processOpType0(uint32 op, State *state);
void processOpType1(uint32 op, State *state);
void processOpType2(uint32 op, State *state);
void processOpType3(uint32 op, State *state);
void processOpType4(uint32 op, State *state);
void processOpType5(uint32 op, State *state);
void processOpType6(uint32 op, State *state);
void processOpType7(uint32 op, State *state);
void processOpType8(uint32 op, State *state);
void processOpType9(uint32 op, State *state);
void processOpTypeA(uint32 op, State *state);
void processOpTypeB(uint32 op, State *state);
void processOpTypeC(uint32 op, State *state);
void processOpTypeD(uint32 op, State *state);
void processOpTypeE(uint32 op, State *state);
void processOpTypeF(uint32 op, State *state);
