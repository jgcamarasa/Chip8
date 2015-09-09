#include "imgui.h"
#include "imgui_impl_glfw.h"

#include "gui.h"


void drawGUI(State *state, Controller *controller)
{
	ImGui_ImplGlfw_NewFrame();
	ImGui::SetNextWindowSize(ImVec2(SIDE_PANEL_W, WIN_H), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(WIN_W - SIDE_PANEL_W * 2, 0));
	ImGui::Begin("Another Window", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("PC: %#010x", state->PC);
	uint32 op;
	readInstruction(&op, state->memory + state->PC);
	char string[200];
	getOpString(op, string);
	ImGui::Text("Op: %#06x", op);
	ImGui::Text("    %s", string);
	ImGui::Separator();
	ImGui::Text("Registers:");
	ImGui::Columns(2);
	ImGui::Text("0:   %d", (int)state->V[0]);
	ImGui::Text("1:   %d", (int)state->V[1]);
	ImGui::Text("2:   %d", (int)state->V[2]);
	ImGui::Text("3:   %d", (int)state->V[3]);
	ImGui::Text("4:   %d", (int)state->V[4]);
	ImGui::Text("5:   %d", (int)state->V[5]);
	ImGui::Text("6:   %d", (int)state->V[6]);
	ImGui::Text("7:   %d", (int)state->V[7]);
	ImGui::NextColumn();
	ImGui::Text("8:   %d", (int)state->V[8]);
	ImGui::Text("9:   %d", (int)state->V[9]);
	ImGui::Text("10:  %d", (int)state->V[10]);
	ImGui::Text("11:  %d", (int)state->V[11]);
	ImGui::Text("12:  %d", (int)state->V[12]);
	ImGui::Text("13:  %d", (int)state->V[13]);
	ImGui::Text("14:  %d", (int)state->V[14]);
	ImGui::Text("15:  %d", (int)state->V[15]);
	ImGui::Columns(1);
	ImGui::Text("I:  %#06x", (int)state->I);
	ImGui::Separator();
	ImGui::Text("Stack:");
	int stackLevel = state->stackLevel;
	for (int i = 0; i < stackLevel; ++i)
	{
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%d:  %#06x", i, state->stack[i]);
	}
	for (int i = stackLevel; i < 8; ++i)
	{
		ImGui::Text("%d:  %#06x", i + stackLevel, state->stack[i + stackLevel]);
	}
	ImGui::Separator();
	if (ImGui::Button(controller->status == CONTROLLER_STATUS_PAUSED ? "Run" : "Pause"))
	{
		if (controller->status == CONTROLLER_STATUS_PAUSED)
		{
			controller->status = CONTROLLER_STATUS_RUNNING;
		}
		else
		{
			controller->status = CONTROLLER_STATUS_PAUSED;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Step"))
	{
		controller->status = CONTROLLER_STATUS_PAUSED;
		doStep(controller->state);
	}
	ImGui::Text("Status: %s", controller->status == CONTROLLER_STATUS_PAUSED ? "Paused" : "Running");
	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(WIN_W - SIDE_PANEL_W*2, 200), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowContentSize(ImVec2(WIN_W - SIDE_PANEL_W*2, 200));
	ImGui::Begin("Memory", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::BeginChildFrame(0, ImVec2(WIN_W - SIDE_PANEL_W*2-20, 160));
	int rows = 256;
	int modifiedCount = 0;
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < 15; j++)
		{
			if (modifiedCount < state->modifiedAddressesCount && state->modifiedAddresses[modifiedCount] == i * 15 + j)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%02X", state->memory[i * 15 + j]);
				modifiedCount++;
			}
			else
			{
				ImGui::Text("%02X", state->memory[i * 15 + j]);
			}
			
			ImGui::SameLine();
		}
		if (modifiedCount < state->modifiedAddressesCount && state->modifiedAddresses[modifiedCount] == i * 15 + 15)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%02X", state->memory[i * 15 + 15]);
			modifiedCount++;
		}
		else
		{
			ImGui::Text("%02X", state->memory[i * 15 + 15]);
		}
		
	}
	
	ImGui::EndChildFrame();
	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(SIDE_PANEL_W, WIN_H), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(WIN_W - SIDE_PANEL_W, 0));
	ImGui::Begin("Code", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	op;
	
	uint32 address = 0x200;
	while (!(state->memory[address] == 0x00 && state->memory[address + 1] == 0x00))
	{
		readInstruction(&op, state->memory + address);
		getOpString(op, string);
		if (state->PC == address)
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%#06x: %s", address, string);
		}
		else
		{
			ImGui::Text("%#06x: %s", address, string);
		}
		
		address += 2;
	}
	

	ImGui::End();
}


void getOpStringType0(uint32 op, char *dest)
{
	switch (op)
	{
	case 0x00E0: // Clears the screen.
		sprintf(dest, "Clear screen");
		break;
	case 0x00EE: // Returns from a subroutine.
		sprintf(dest, "Return from subroutine");
		break;
	case PROGRAM_END:
		break;
	default:
		sprintf(dest, "UNSUPPORTED");
	}
}

void getOpStringType1(uint32 op, char *dest)
{
	// 0x1NNN - Jumps to address NNN.
	uint32 address = op & 0x0000FFF;
	sprintf(dest, "Jump to %#06x", address);
}

void getOpStringType2(uint32 op, char *dest)
{
	// 0x2NNN - Calls subroutine at NNN.
	uint32 value = op & 0x0FFF;
	sprintf(dest, "Call %#06x", value);
}

void getOpStringType3(uint32 op, char *dest)
{
	// 0x3XNN - Skips the next instruction if VX equals NN.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 value = op & 0x00FF;
	sprintf(dest, "Skip if V%d==%d", X, value);
}

void getOpStringType4(uint32 op, char *dest)
{
	// 0x4XNN - Skips the next instruction if VX doesn't equal NN.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 value = op & 0x00FF;
	sprintf(dest, "Skip if V%d!=%d", X, value);
}

void getOpStringType5(uint32 op, char *dest)
{
	// 0x5XY0 - Skips the next instruction if VX equals VY.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 Y = op & 0x00F0;
	Y = Y >> 4;
	sprintf(dest, "Skip if V%d==V%d", X, Y);
}

void getOpStringType6(uint32 op, char *dest)
{
	// 0x6XNN - Sets VX to NN.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 value = op & 0x00FF;
	sprintf(dest, "V%d=%d", X, value);
}

void getOpStringType7(uint32 op, char *dest)
{
	// 0x7XNN - Adds NN to VX.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 value = op & 0x00FF;
	sprintf(dest, "V%d+=%d", X, value);
}

void getOpStringType8(uint32 op, char *dest)
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
		sprintf(dest, "V%d=V%d", X, Y);
		break;
	case 0x1: // Sets VX to VX or VY. (bitwise)
		sprintf(dest, "V%d|=V%d", X, Y);
		break;
	case 0x2: // Sets VX to VX and VY. (bitwise)
		sprintf(dest, "V%d&=V%d", X, Y);
		break;
	case 0x3: // Sets VX to VX xor VY. (bitwise)
		sprintf(dest, "V%d^=V%d", X, Y);
		break;
	case 0x4: // Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
		sprintf(dest, "V%d+=V%d", X, Y);
		break;
	case 0x5: // VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		sprintf(dest, "V%d-=V%d", X, Y);
		break;
	case 0x6: // Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
		sprintf(dest, "V%d>>=1");
		break;
	case 0x7: // Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		sprintf(dest, "V%d=V%d-V%d", X, Y, X);
		break;
	case 0xE: // Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
		sprintf(dest, "V%d<<=1");
		break;
	default:
		sprintf(dest, "UNSUPPORTED");
		//assert(false);
		break;
	}

}

void getOpStringType9(uint32 op, char *dest)
{
	// 0x9XY0 - Skips the next instruction if VX doesn't equal VY.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 Y = op & 0x00F0;
	Y = Y >> 4;
	sprintf(dest, "Skip if V%d!=V%d", X, Y);
}

void getOpStringTypeA(uint32 op, char *dest)
{
	// 0xANNN - Sets I to the address NNN.
	uint32 address = op & 0x0FFF;
	sprintf(dest, "I=%#06x", address);
}

void getOpStringTypeB(uint32 op, char *dest)
{
	// 0xBNNN - Jumps to the address NNN plus V0.
	uint32 address = op & 0x0FFF;
	sprintf(dest, "Jump to V%#06x+V0", address);
}

void getOpStringTypeC(uint32 op, char *dest)
{
	// 0xCXNN - Sets VX to the result of a bitwise and operation on a random number and NN.
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 value = op & 0x00FF;
	uint32 r = rand();
	r &= 0x00FF; // Just in case
	sprintf(dest, "V%d = %d(rand)|%d", X, r, value);
}

void getOpStringTypeD(uint32 op, char *dest)
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
	sprintf(dest, "Print sprite in I at V%d, V%d", X, Y);
}

void getOpStringTypeE(uint32 op, char *dest)
{
	// 0xE??? - Keyboard skips
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 subtype = op & 0x00FF;
	switch (subtype)
	{
	case 0x9E: // Skips the next instruction if the key stored in VX is pressed.
		sprintf(dest, "Skip if key at V%d pressed", X);
		break;
	case 0xA1: // Skips the next instruction if the key stored in VX isn't pressed.
		sprintf(dest, "Skip if key at V%d not pressed", X);
		break;
	default:
		sprintf(dest, "UNSUPPORTED");
		//assert(false);
	}
}

void getOpStringTypeF(uint32 op, char *dest)
{
	uint32 X = op & 0x0F00;
	X = X >> 8;
	uint32 subtype = op & 0x00FF;
	switch (subtype)
	{
	case 0x07: // Sets VX to the value of the delay timer.
		sprintf(dest, "V%d=Delay", X);
		break;
	case 0x0A: // A key press is awaited, and then stored in VX.
		sprintf(dest, "Waiting for press (V%d)", X);
		break;
	case 0x15: // Sets the delay timer to VX.
		sprintf(dest, "Delay=V%d", X);
		break;
	case 0x18: // Sets the sound timer to VX.
		sprintf(dest, "Sound=V%d", X);
		break;
	case 0x1E: // Adds VX to I
		sprintf(dest, "I+=V%d", X);
		break;
	case 0x29: // Sets I to the location of the sprite for the character in VX. 
		// Characters 0-F (in hexadecimal) are represented by a 4x5 font.
		sprintf(dest, "I=char in V%d", X);
		break;
	case 0x33: // Stores the Binary-coded decimal representation of VX, with the most 
		// significant of three digits at the address in I, the middle digit 
		// at I plus 1, and the least significant digit at I plus 2. 
		// (In other words, take the decimal representation of VX, place the 
		// hundreds digit in memory at location in I, the tens digit at location I+1, 
		// and the ones digit at location I+2.)
		sprintf(dest, "BCD of V%d", X);
		break;
	case 0x55: // Stores V0 to VX in memory starting at address I.
		sprintf(dest, "Write V0-V%d at I", X);
		break;
	case 0x65: // Fills V0 to VX with values from memory starting at address I.
		sprintf(dest, "Read V0-V%d from I", X);
		break;
	default:
		sprintf(dest, "UNSUPPORTED");
	}
}

void getOpString(uint32 op, char *dest)
{
	uint32 opType;
	getOpType(&opType, op);
	switch (opType)
	{
	case 0x0000:
		getOpStringType0(op, dest);
		break;
	case 0x1000:
		getOpStringType1(op, dest);
		break;
	case 0x2000:
		getOpStringType2(op, dest);
		break;
	case 0x3000:
		getOpStringType3(op, dest);
		break;
	case 0x4000:
		getOpStringType4(op, dest);
		break;
	case 0x5000:
		getOpStringType5(op, dest);
		break;
	case 0x6000:
		getOpStringType6(op, dest);
		break;
	case 0x7000:
		getOpStringType7(op, dest);
		break;
	case 0x8000:
		getOpStringType8(op, dest);
		break;
	case 0x9000:
		getOpStringType9(op, dest);
		break;
	case 0xA000:
		getOpStringTypeA(op, dest);
		break;
	case 0xB000:
		getOpStringTypeB(op, dest);
		break;
	case 0xC000:
		getOpStringTypeC(op, dest);
		break;
	case 0xD000:
		getOpStringTypeD(op, dest);
		break;
	case 0xE000:
		getOpStringTypeE(op, dest);
		break;
	case 0xF000:
		getOpStringTypeF(op, dest);
		break;
	default:
		sprintf(dest, "UNSUPPORTED");
		break;
	}
}
