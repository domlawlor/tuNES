#pragma once

#include "globals.h"

#include "raylib.h"

#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "cartridge.h"

constexpr u16 romFileNameMaxLength = Kilobytes(1);

constexpr u8 gWindowScale = 4.0f;
constexpr u16 gNesWidth = 256;
constexpr u16 gNesHeight = 240;
constexpr u32 gWindowWidth = gNesWidth * gWindowScale;
constexpr u32 gWindowHeight = gNesHeight * gWindowScale;

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
	bool pad1Buttons[BUTTON_NUM] = {};
	bool pad2Buttons[BUTTON_NUM] = {};

	bool padStrobe = false;
	u8 pad1CurrentButton = 0;
	u8 pad2CurrentButton = 0;
	//u8 inputBusBuffer = 0;
};

class Nes
{
public:
	void Init();
	void Deinit();

	void Update();

	static Cpu *GetCpu() { return GetInstance().m_cpu; };
	static Ppu *GetPpu() { return GetInstance().m_ppu; };
	static Apu *GetApu() { return GetInstance().m_apu; };
	static Cartridge *GetCartridge() { return GetInstance().m_cartridge; };
	static Input *GetInput() { return &GetInstance().m_input; };

	static Nes &GetInstance() { return _instance; };

	static void PowerButton() { GetInstance().m_triggerPowerOn = true; }
	static void ResetButton() { GetInstance().m_triggerReset = true; }
	static void QueueRomLoad(char *loadFileName) { GetInstance().QueueRomLoadInternal(loadFileName); };

	static bool HasRomLoaded() { return GetInstance().m_cartridge != nullptr; }

	static void SetOpenBus(u8 value) { GetInstance().openBus = value; }
	static u8 GetOpenBus() { return GetInstance().openBus; }
private:
	void Reset(bool cyclePower);

	void QueueRomLoadInternal(char *loadFileName);

	bool LoadCartridge(const char *fileName);

	void InputUpdate();

	static Nes _instance;

	Cpu *m_cpu;
	Ppu *m_ppu;
	Apu *m_apu;
	Cartridge *m_cartridge;
	Input m_input;

	u8 openBus = 0;

	bool m_triggerPowerOn = false;
	bool m_triggerReset = false;
	bool m_isPowerOn = false;

	char m_romFileName[romFileNameMaxLength];
	bool m_triggerRomLoad = false;
};