/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "nes.h"

#include "mapper.cpp"
#include "interrupts.cpp"
#include "memory.cpp"

#include "apu.cpp"
#include "ppu.cpp"
#include "cpu.cpp"

void Nes::Update()
{
	InputFrame();

	if(hitPowerOn)
	{
		hitPowerOn = false;
		Power();
	}
	if(hitReset)
	{
		hitReset = false;
		LoadCartridge(romFileName);
		Reset();
	}

	// TODO: If power is off, could we just run the nes on an empty cartridge?
	//       Or is this the best check to stop extra work being done
	if(!isPowerOn)
	{
		return;
	}

	ppu.hitEndFrame = false;
	while(!ppu->hitEndFrame)
	{
		// Run Ppu
		// Run Cpu
		// Run Ppu again??
	}

	// Input read // TODO: Only run when reading input??
	// TODO: Move this to where it happens in memory read.
	if(cpu.padStrobe)
	{
		for(u8 idx = 0; idx < Input::BUTTON_NUM; ++idx)
		{
			cpu.inputPad1.buttons[idx] = input.buttons[idx];
		}
	}

	nes->frameClocksElapsed += RunCpu(cpu, newInput);


	Texture2D drawTexture = backBuffers[currentDrawBackBufferIndex];
	currentDrawBackBufferIndex = (currentDrawBackBufferIndex + 1) % backBufferCount;
	UpdateTexture(drawTexture, ppu.pixelBuffer);

	BeginDrawing();
	ClearBackground(GREEN);
	DrawTexture(drawTexture, 0, 0, WHITE);
	EndDrawing();
}

void Nes::LoadCartridge(u8 *fileName)
{
	Cartridge *cartridge = &nes->cartridge;
	Cpu *cpu = &nes->cpu;
	Ppu *ppu = &nes->ppu;

	// Reading rom file
	cartridge->fileName = fileName;
	cartridge->fileSize;
	//cartridge->data = (u8 *)ReadFileData(fileName, &cartridge->fileSize);

	if(cartridge->fileSize == 0)
	{
		nes->powerOn = false;
		return;
	}
	else
	{
		nes->powerOn = true;

		u8 *romData = cartridge->data;

		// NOTE: Check for correct header
		Assert(romData[0] == 'N' && romData[1] == 'E' && romData[2] == 'S' && romData[3] == 0x1A);

		// NOTE: Read header
		cartridge->prgBankCount = romData[4];
		cartridge->chrBankCount = romData[5];
		u8 flags6 = romData[6];
		u8 flags7 = romData[7];
		cartridge->prgRamSize = romData[8];

		cartridge->useVertMirror = (flags6 & (1)) != 0;
		cartridge->hasBatteryRam = (flags6 & (1 << 1)) != 0;
		cartridge->hasTrainer = (flags6 & (1 << 2)) != 0;
		cartridge->useFourScreenMirror = (flags6 & (1 << 3)) != 0;
		cartridge->mapperNum = (flags7 & 0xF0) | (flags6 >> 4);

		Assert(cartridge->useFourScreenMirror == 0);

		if(cartridge->useFourScreenMirror) {
			nes->ppu.mirrorType = FOUR_SCREEN_MIRROR;
		}
		else if(cartridge->useVertMirror) {
			nes->ppu.mirrorType = VERTICAL_MIRROR;
		}
		else {
			nes->ppu.mirrorType = HORIZONTAL_MIRROR;
		}

		cartridge->prgData = romData + 16; // PrgData starts after the header info(16 bytes)

		if(cartridge->hasTrainer)
		{
			cartridge->prgData += 512; // Trainer size 512 bytes
		}

		cartridge->chrData = cartridge->prgData + (cartridge->prgBankCount * Kilobytes(16));

		Assert(cartridge->mapperNum < ArrayCount(MapperInit));
		MapperInit[cartridge->mapperNum](cartridge, cpu, ppu);
	}
}

void Nes::Power()
{
	isPowerOn = !isPowerOn;

	if(isPowerOn)
	{
		LoadCartridge(romFileName);
	}

	cpu.InitCpu();
	InitPpu();
}

void Nes::Reset()
{
	InitCpu(&cpu);
	InitPpu(&ppu);

	LoadCartridge(romFileName);

	// NOTE: The status after reset was taken from nesdev
	cpu.stackPointer -= 3;
	SetInterrupt(&cpu.flags);

	cpu.prgCounter = ReadCpu16(RESET_VEC, &cpu);
}

void Nes::InputFrame()
{
	bool altDown = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
	bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

	input.buttons[Input::B_UP] = IsKeyDown(KEY_UP);
	input.buttons[Input::B_DOWN] = IsKeyDown(KEY_DOWN);
	input.buttons[Input::B_LEFT] = IsKeyDown(KEY_LEFT);
	input.buttons[Input::B_RIGHT] = IsKeyDown(KEY_RIGHT);

	input.buttons[Input::B_A] = IsKeyDown(KEY_Z);
	input.buttons[Input::B_B] = IsKeyDown(KEY_X);
	input.buttons[Input::B_START] = IsKeyDown(KEY_ENTER);
	input.buttons[Input::B_SELECT] = shiftDown;
}