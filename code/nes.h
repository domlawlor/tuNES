#pragma once

#include "globals.h"

#include "raylib.h"

#include "cpu.h"
#include "ppu.h"
//#include "apu.h"

//#include <windows.h>

struct Input
{
	enum Buttons {
		B_A = 0,
		B_B,
		B_SELECT,
		B_START,
		B_UP,
		B_DOWN,
		B_LEFT,
		B_RIGHT,
		BUTTON_NUM
	};
	bool buttons[BUTTON_NUM];
};

//static const u8 MapperTotal = 8;

struct iNESHeader
{
	u8 nesId[4];
	u8 prgBankCount;
	u8 chrBankCount;
	u8 flags6;
	u8 flags7;
	u8 prgRamSize;
	u8 flags9;
	u8 flags10;
	u8 unusedPadding[5];
};
static_assert(sizeof(iNESHeader) == 16, "iNESHeader must always be 16 bytes");


enum class NametableMirror
{
	SINGLE_SCREEN_BANK_A = 0,
	SINGLE_SCREEN_BANK_B,
	VERTICAL_MIRROR,
	HORIZONTAL_MIRROR,
	FOUR_SCREEN_MIRROR,
};

struct Cartridge
{
	u8 *fileName;
	u8 *romFileData;
	u32 romFileSize;

	u8 *prgBanks;
	u8 *chrBanks;

	u8 prgBankCount;
	u8 chrBankCount;
	u8 prgRamSize;

	bool useVerticalMirror;
	bool hasBatteryRam;
	bool hasTrainer;
	bool useFourScreenMirror;

	u8 mapperNum;

	NametableMirror nametableMirrorType;

	// u16 extRegister;
	// u8 mapperInternalReg;
	// u8 mapperWriteCount;

	//	u8 prgRomMode;
	//	bool chr4KbMode;
};

inline void MemorySet(void *memory, u8 value, u64 size)
{
	u8 *memoryU8 = (u8 *)memory;
	for(u64 i = 0; i < size; ++i)
	{
		memoryU8[i] = value;
	}
}

// Copy Size amount of bytes from source to destination
inline void MemoryCopy(u8 *dest, u8 *src, u16 size)
{
	// NOTE: Very basic copy. Not bounds protection
	for(u16 byte = 0; byte < size; ++byte)
	{
		dest[byte] = src[byte];
	}
}

class Nes
{
public:
	void Update();

	//static Cpu *GetCpu() { return &GetInstance().cpu; };
	static Ppu *GetPpu() { return &GetInstance().ppu; };
	static Cartridge *GetCartridge() { return &GetInstance().cartridge; };
	//static Input *GetInput() { return &GetInstance().input; };

	static Nes &GetInstance() { return instance; };
private:
	void Power();
	void Reset();

	void LoadCartridge(u8 *romFileName);
	void InputFrame();

	static Nes instance;

	Cpu cpu;
	Ppu ppu;
	Cartridge cartridge;
	Input input;

	bool hitPowerOn;
	bool hitReset;
	bool isPowerOn;
};



//struct Nes
//{
//	Cpu cpu;
//	Ppu ppu;
//	Apu apu;
//	Cartridge cartridge;
//
//	u8 *cpuMemoryBase;
//	u8 *ppuMemoryBase;
//
//	bool powerOn;
//
//	r32 cpuHz;
//
//	r32 frameClocksElapsed;
//	r32 frameClockTotal;
//
//	u8 openBus;
//};

//global Nes globalNes = {};
//global u32 *globalScreenPixelPtr = nullptr;