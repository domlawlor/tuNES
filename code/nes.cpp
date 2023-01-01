/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "nes.h"

#include "cpu.cpp"
#include "ppu.cpp"
#include "apu.cpp"
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

			u64 currentCycle = m_cpu->GetCycleNum();
			u64 cyclesElapsed = (currentCycle - m_cyclesStartHz);
			if(cyclesElapsed >= (gNesCpuClockRate/120))
			{
				r64 currentTime = GetTime();
				r64 deltaTime = currentTime - m_startHzTime;
				if(deltaTime < (1.0/120))
				{
					continue;
				}
				else
				{
					m_cyclesStartHz = currentCycle;
					m_startHzTime = currentTime;
				}
			}

			m_cpu->Run();
			newFrameHit = m_ppu->GetFrameNum() != currentFrameNum;
		}
	}
}

void Nes::Init()
{
	m_cpu = new Cpu();
	m_ppu = new Ppu();
	m_apu = new Apu();

	m_startHzTime = GetTime();
}

void Nes::Deinit()
{
	delete m_cpu;
	delete m_ppu;
	delete m_apu;
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
	
	if(IsFileExtension(fileName, ".nes"))
	{
		m_cartridge = Cartridge::CreateCartridgeForRom(romFileData, romFileSize);
		if(!m_cartridge)
		{
			TraceLog(LOG_ERROR, "LoadCartridge failed to CreateCartridgeForRom for file - %s", fileName);
			return false;
		}
		m_nesMode = NesMode::ROM;
	}
	else if(IsFileExtension(fileName, ".nsf"))
	{
		struct NSFHeader
		{
			u8 nesId[5];
			u8 versionNumber;
			u8 totalSongs;
			u8 startingSong;

			u16 loadAddress;
			u16 initAddress;
			u16 playAddress;

			u8 songName[32];
			u8 artistName[32];
			u8 copyright[32];

			u16 playSpeedNtsc;
			u8 bankswitchInitValues[8];
			u16 playSpeedPal;
			u8 regionByte;
			u8 soundChipsUsed;
			u8 reservedNSF2;
			u8 programDataLength[3];
		};
		Assert(sizeof(NSFHeader) == 0x80);

		NSFHeader *header = (NSFHeader *)romFileData;

		bool isNsfRom = (header->nesId[0] == 'N' && header->nesId[1] == 'E'
			&& header->nesId[2] == 'S' && header->nesId[2] == 'M');

		//TraceLog(LOG_ERROR, "LoadCartridge failed. NSF not implemented - %s", fileName);
		//return false;

		m_nesMode = NesMode::NSF;
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