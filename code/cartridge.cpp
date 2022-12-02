#include "nes.h"

bool Cartridge::LoadCartridge(const char *fileName)
{
	u32 romFileSize = 0;
	u8 *romFileData = LoadFileData((const char *)fileName, &romFileSize);

	if(!romFileData)
	{
		return false;
	}

	iNESHeader *romHeader = (iNESHeader *)romFileData;

	bool isINesRom = (romHeader->nesId[0] == 'N' && romHeader->nesId[1] == 'E' &&
		romHeader->nesId[2] == 'S' && romHeader->nesId[3] == '\0');

	if(!isINesRom)
	{
		return false;
	}

	// Now get the correct mapper class
	u8 mapperNum = (romHeader->flags7 & 0xF0) | (romHeader->flags6 >> 4);
	Cartridge *cartridgeImpl = GetCartridgeByMapper(mapperNum);

	if(!cartridgeImpl)
	{
		TraceLog(LOG_ERROR, "LoadCartridge failed - Mapper Num %u does not have implementation", mapperNum);
		return false;
	}

	// We have a rom to load, and have a mppaer. so clear the old one here
	if(instance)
	{
		instance->ClearCartridge();
		delete instance;
	}

	instance = cartridgeImpl;
	instance->ParseRom(fileName, romFileData, romFileSize);
	instance->Init();
}

void Cartridge::ParseRom(const char *romFileName, u8* romData, u32 romSize)
{
	m_fileName = romFileName;

	iNESHeader *romHeader = (iNESHeader *)romData;

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

	m_useVerticalMirror = (romHeader->flags6 & (1)) != 0;
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

void Cartridge::ClearCartridge()
{
	if(m_romFileData)
	{
		UnloadFileData(m_romFileData);
	}
	*this = {};
}
