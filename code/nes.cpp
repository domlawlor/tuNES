/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "nes.h"

//#include "memory.cpp"

#include "cpu.cpp"
#include "ppu.cpp"
//#include "apu.cpp"
#include "cartridge.cpp"

void Nes::Update()
{
	InputFrame();

	if(m_hitPowerOn)
	{
		m_hitPowerOn = false;
		Power();
	}
	if(m_hitReset)
	{
		m_hitReset = false;
		m_cartridge.LoadCartridge(m_romFileName);
		Reset();
	}

	// TODO: If power is off, could we just run the nes on an empty cartridge?
	//       Or is this the best check to stop extra work being done
	if(!m_isPowerOn)
	{
		return;
	}

	m_ppu.hitEndFrame = false;
	while(!m_ppu.hitEndFrame)
	{
		// Run Ppu
		// Run Cpu
		// Run Ppu again??
	}

	// Input read // TODO: Only run when reading input??
	// TODO: Move this to where it happens in memory read.
	/*if(m_cpu.padStrobe)
	{
		for(u8 idx = 0; idx < Input::BUTTON_NUM; ++idx)
		{
			m_cpu.inputPad1.buttons[idx] = m_input.buttons[idx];
		}
	}*/

	m_cpu.Run();


	
}

void Nes::Init()
{
	// cartridge.LoadCartridge("Mario Bros.nes");
	m_cartridge.LoadCartridge(m_romFileName);
	Power();
}

void Nes::Power()
{
	m_isPowerOn = !m_isPowerOn;

	if(!m_isPowerOn)
	{
		return;
	}
	
	if(!m_cartridge.LoadCartridge(m_romFileName))
	{
		m_isPowerOn = false;
		return;
	}
	
	m_cpu.Init();
	m_ppu.Init();
}

void Nes::Reset()
{
	if(!m_cartridge.LoadCartridge(m_romFileName))
	{
		m_isPowerOn = false;
		return;
	}

	m_cpu.Init();
	m_ppu.Init();

	m_cpu.OnReset();
}

void Nes::InputFrame()
{
	bool altDown = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
	bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

	m_input.buttons[Input::B_UP] = IsKeyDown(KEY_UP);
	m_input.buttons[Input::B_DOWN] = IsKeyDown(KEY_DOWN);
	m_input.buttons[Input::B_LEFT] = IsKeyDown(KEY_LEFT);
	m_input.buttons[Input::B_RIGHT] = IsKeyDown(KEY_RIGHT);

	m_input.buttons[Input::B_A] = IsKeyDown(KEY_Z);
	m_input.buttons[Input::B_B] = IsKeyDown(KEY_X);
	m_input.buttons[Input::B_START] = IsKeyDown(KEY_ENTER);
	m_input.buttons[Input::B_SELECT] = shiftDown;
}