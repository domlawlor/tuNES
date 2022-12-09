/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "nes.h"

#include "cpu.cpp"
#include "ppu.cpp"
//#include "apu.cpp"
#include "cartridge.cpp"

// Static define
Nes Nes::_instance;

void Nes::Update()
{
	if(m_triggerPowerOn && m_isPowerOn)
	{
		m_triggerPowerOn = false;
		m_isPowerOn = false;
		return;
	}

	if(m_triggerRomLoad || m_triggerPowerOn || m_triggerReset)
	{
		bool cyclePower = m_triggerRomLoad || m_triggerPowerOn;
		Reset(cyclePower);
		
		m_triggerRomLoad = false;
		m_triggerPowerOn = false;
		m_triggerReset = false;
	}

	if(m_isPowerOn)
	{
		u64 currentFrameNum = m_ppu->GetFrameNum();
		bool newFrameHit = false;
		while(!newFrameHit)
		{
			InputUpdate();
			m_cpu->Run();
			newFrameHit = m_ppu->GetFrameNum() != currentFrameNum;
		}
	}
}

void Nes::Init()
{
	m_cpu = new Cpu();
	m_ppu = new Ppu();
}

void Nes::Deinit()
{
	delete m_cpu;

	//m_ppu->Deinit();
	delete m_ppu;
	delete m_cartridge;
}

void Nes::Reset(bool cyclePower)
{
	if(LoadCartridge(m_romFileName))
	{
		m_ppu->Reset();
		m_cpu->Reset(cyclePower);

		m_isPowerOn = true;
	}
	else
	{
		TraceLog(LOG_ERROR, "LoadCartridge failed on file - %s", m_romFileName);
		m_isPowerOn = false;
		return;
	}
}

void Nes::QueueRomLoadInternal(char *loadFileName)
{
	TextCopy(m_romFileName, loadFileName);
	m_triggerRomLoad = true;
}

bool Nes::LoadCartridge(const char *fileName)
{
	u32 romFileSize = 0;
	u8 *romFileData = LoadFileData((const char *)fileName, &romFileSize);
	if(!romFileData)
	{
		TraceLog(LOG_ERROR, "LoadFileData failed for filePath - %s", fileName);
		return false;
	}

	if(m_cartridge)
	{
		delete m_cartridge;
		m_cartridge = nullptr;
	}

	m_cartridge = Cartridge::CreateCartridgeForRom(romFileData, romFileSize);

	if(!m_cartridge)
	{
		TraceLog(LOG_ERROR, "LoadCartridge failed to CreateCartridgeForRom for file - %s", fileName);
		return false;
	}

	return true;
}


void Nes::InputUpdate()
{
	if(m_input.padStrobe)
	{
		bool altDown = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
		bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);


		m_input.pad1Buttons[Input::B_UP] = IsKeyDown(KEY_UP);
		m_input.pad1Buttons[Input::B_DOWN] = IsKeyDown(KEY_DOWN);
		m_input.pad1Buttons[Input::B_LEFT] = IsKeyDown(KEY_LEFT);
		m_input.pad1Buttons[Input::B_RIGHT] = IsKeyDown(KEY_RIGHT);

		m_input.pad1Buttons[Input::B_A] = IsKeyDown(KEY_Z);
		m_input.pad1Buttons[Input::B_B] = IsKeyDown(KEY_X);
		m_input.pad1Buttons[Input::B_START] = IsKeyDown(KEY_ENTER);
		m_input.pad1Buttons[Input::B_SELECT] = shiftDown;

		m_input.pad1CurrentButton = Input::B_A;
		m_input.pad2CurrentButton = Input::B_A;

		
		//TraceLog(LOG_INFO, "Input: A=%d, B=%d Start=%d Select=%d Up=%d Down=%d Left=%d Right=%d",
		//	m_input.pad1Buttons[Input::B_A], m_input.pad1Buttons[Input::B_B], m_input.pad1Buttons[Input::B_START], m_input.pad1Buttons[Input::B_SELECT],
		//	m_input.pad1Buttons[Input::B_UP], m_input.pad1Buttons[Input::B_DOWN], m_input.pad1Buttons[Input::B_LEFT], m_input.pad1Buttons[Input::B_RIGHT]);
	}
}