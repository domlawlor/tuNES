#pragma once

#include "globals.h"

#include "raylib.h"

#include "cpu.h"
#include "ppu.h"
//#include "apu.h"
#include "cartridge.h"

constexpr u8 gWindowScale = 1.0f;
constexpr u16 gNesWidth = 256;
constexpr u16 gNesHeight = 240;
constexpr u32 gWindowWidth = gNesWidth * gWindowScale;
constexpr u32 gWindowHeight = gNesHeight * gWindowScale;

inline void MemorySet(void *memory, u8 value, u64 size)
{
	u8 *memoryU8 = (u8 *)memory;
	for(u64 i = 0; i < size; ++i)
	{
		memoryU8[i] = value;
	}
}

/*
// Copy Size amount of bytes from source to destination
inline void MemoryCopy(u8 *dest, u8 *src, u16 size)
{
	// NOTE: Very basic copy. Not bounds protection
	for(u16 byte = 0; byte < size; ++byte)
	{
		dest[byte] = src[byte];
	}
}
*/

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

class Nes
{
public:
	void Init();
	void Update();

	static Cpu *GetCpu() { return &GetInstance().m_cpu; };
	static Ppu *GetPpu() { return &GetInstance().m_ppu; };
	static Cartridge *GetCartridge() { return &GetInstance().m_cartridge; };
	//static Input *GetInput() { return &GetInstance().input; };

	static Nes &GetInstance() { return _instance; };
private:
	void Power();
	void Reset();

	void LoadCartridge(u8 *romFileName);
	void InputFrame();

	static Nes _instance;

	Cpu m_cpu;
	Ppu m_ppu;
	Cartridge m_cartridge;
	Input m_input;

	bool m_hitPowerOn;
	bool m_hitReset;
	bool m_isPowerOn;

	const char *m_romFileName = "Mario Bros.nes";
};

//struct Nes
//{
//	r32 cpuHz;
//	u8 openBus;
//};