#include "nes.h"

Cartridge *Cartridge::CreateCartridgeForRom(u8 *romData, u32 romSize)
{
	iNESHeader *romHeader = (iNESHeader *)romData;

	bool isINesRom = (romHeader->nesId[0] == 'N' && romHeader->nesId[1] == 'E' && romHeader->nesId[2] == 'S');

	if(!isINesRom)
	{
		return nullptr;
	}

	// Now get the correct mapper class
	u8 mapperNum = (romHeader->flags7 & 0xF0) | (romHeader->flags6 >> 4);

	Cartridge *cartridge = nullptr;


	switch(mapperNum)
	{
	case 0: cartridge = new NROM(); break;
	case 1: cartridge = new MMC1(); break;
	case 2: cartridge = new UNROM(); break;
	case 3: cartridge = new CNROM(); break;
	case 7: cartridge = new AXROM(); break;
	default: TraceLog(LOG_ERROR, "Missing mapper impl for mapper num - %u", mapperNum); return nullptr; break;
	};

	cartridge->ParseRom(romData, romSize);
	cartridge->Init();

	return cartridge;
}

void Cartridge::ParseRom(u8* romData, u32 romSize)
{
	iNESHeader *romHeader = (iNESHeader *)romData;

	m_romFileData = romData;
	m_romFileSize = romSize;

	m_prgBankCount = romHeader->prgBankCount;
	m_chrBankCount = romHeader->chrBankCount;
	m_prgRamSize = romHeader->prgRamSize;

	m_prgBanks = romData + sizeof(iNESHeader);

	if(m_hasTrainer)
	{
		constexpr u16 trainerSize = 512;
		m_prgBanks += trainerSize;
	}

	constexpr u32 prgBankSize = Kilobytes(16);
	m_chrBanks = m_prgBanks + (m_prgBankCount * prgBankSize);

	m_useVerticalMirror = (romHeader->flags6 & 0x01) != 0;
	m_hasBatteryRam = (romHeader->flags6 & (1 << 1)) != 0;
	m_hasTrainer = (romHeader->flags6 & (1 << 2)) != 0;
	m_useFourScreenMirror = (romHeader->flags6 & (1 << 3)) != 0;
	AssertMsg(m_useFourScreenMirror == 0, "Not implemented yet");

	m_mapperNum = (romHeader->flags7 & 0xF0) | (romHeader->flags6 >> 4);

	if(m_useFourScreenMirror)
	{
		m_nametableMirrorType = NametableMirror::FOUR_SCREEN_MIRROR;
	}
	else if(m_useVerticalMirror)
	{
		m_nametableMirrorType = NametableMirror::VERTICAL_MIRROR;
	}
	else
	{
		m_nametableMirrorType = NametableMirror::HORIZONTAL_MIRROR;
	}
}
