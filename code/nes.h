#pragma once

#include "globals.h"

#include "cpu.h"
#include "ppu.h"
#include "apu.h"

#include <windows.h>

static const u8 MapperTotal = 8;

struct Cartridge
{
	u8 *fileName;
	u32 fileSize;
	u8 *data;

	u8 prgBankCount;
	u8 chrBankCount;

	u8 *prgData;
	u8 *chrData;

	u8 prgRamSize;

	u8 mapperNum;

	u16 extRegister;

	bool useVertMirror;
	bool hasBatteryRam;
	bool hasTrainer;
	bool useFourScreenMirror;

	u8 mapperInternalReg;
	u8 mapperWriteCount;

	u8 prgRomMode;
	bool chr4KbMode;
};

struct Nes
{
	Cpu cpu;
	Ppu ppu;
	Apu apu;
	Cartridge cartridge;

	u8 *cpuMemoryBase;
	u8 *ppuMemoryBase;

	bool powerOn;

	r32 cpuHz;

	r32 frameClocksElapsed;
	r32 frameClockTotal;

	u8 openBus;

#if CPU_LOG
	HANDLE logCpuHandle;
#endif
};

global Nes globalNes = {};
global u32 *globalScreenPixelPtr = nullptr;