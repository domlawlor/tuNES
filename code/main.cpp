
#include "globals.h"

#include <cstdio> // for printf
#include <stdlib.h>

#include "nes.h"

#include "mapper.cpp"

constexpr u8 backBufferCount = 2;
Texture2D backBuffers[backBufferCount] = {};
u8 currentDrawBackBufferIndex = 0;

void InitialiseBackBuffers(u32 bufferWidth, u32 bufferHeight)
{
	// Throw away image for generating the texture2d's
	Image image = {};
	image.data = MemAlloc(bufferWidth * bufferHeight * sizeof(Color));
	image.width = bufferWidth;
	image.height = bufferHeight;
	image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	image.mipmaps = 1;

	for(int i = 0; i < backBufferCount; ++i)
	{
		backBuffers[i] = LoadTextureFromImage(image);
	}
}

void ClearCartridge(Cartridge *cartridge)
{
	if(cartridge->romFileData)
	{
		UnloadFileData(cartridge->romFileData);
	}

	*cartridge = {};
}

void LoadCartridge(const char *fileName, Cartridge *cartridge)
{
	ClearCartridge(cartridge);

	// Read nes rom
	u8 *romFileData = LoadFileData(fileName, &cartridge->romFileSize);

	cartridge->romFileData = romFileData;

	iNESHeader *romHeader = (iNESHeader *)romFileData;
	Assert(romHeader->nesId[0] == 'N' && romHeader->nesId[1] == 'E' &&
		romHeader->nesId[2] == 'S' && romHeader->nesId[3] == '\0');

	cartridge->prgBankCount = romHeader->prgBankCount;
	cartridge->chrBankCount = romHeader->chrBankCount;
	cartridge->prgRamSize = romHeader->prgRamSize;

	cartridge->prgBanks = romFileData + sizeof(iNESHeader);

	if(cartridge->hasTrainer)
	{
		constexpr u16 trainerSize = 512;
		cartridge->prgBanks += trainerSize;
	}

	constexpr u32 prgBankSize = Kilobytes(16);
	cartridge->chrBanks = cartridge->prgBanks + (cartridge->prgBankCount * prgBankSize);

	cartridge->useVerticalMirror = (romHeader->flags6 & (1)) != 0;
	cartridge->hasBatteryRam = (romHeader->flags6 & (1 << 1)) != 0;
	cartridge->hasTrainer = (romHeader->flags6 & (1 << 2)) != 0;
	cartridge->useFourScreenMirror = (romHeader->flags6 & (1 << 3)) != 0;
	AssertMsg(cartridge->useFourScreenMirror == 0, "Not implemented yet");

	cartridge->mapperNum = (romHeader->flags7 & 0xF0) | (romHeader->flags6 >> 4);

	if(cartridge->useFourScreenMirror)
	{
		cartridge->nametableMirrorType = NametableMirror::FOUR_SCREEN_MIRROR;
	}
	else if(cartridge->useVerticalMirror)
	{
		cartridge->nametableMirrorType = NametableMirror::VERTICAL_MIRROR;
	}
	else
	{
		cartridge->nametableMirrorType = NametableMirror::HORIZONTAL_MIRROR;
	}

	Assert(cartridge->mapperNum < ArrayCount(MapperInit));
	MapperInit[cartridge->mapperNum](&cartridge, &cpu, &ppu);
}


static u8 *GetNametableBank(u16 address, Ppu *ppu, Cartridge *cartridge)
{
	u8 *result = 0;

	switch(cartridge->nametableMirrorType)
	{
	case NametableMirror::SINGLE_SCREEN_BANK_A:
		result = ppu->nametableBankA;
		break;
	case NametableMirror::SINGLE_SCREEN_BANK_B:
		result = ppu->nametableBankB;
		break;
	case NametableMirror::VERTICAL_MIRROR:
		if(address < 0x2400 || (0x2800 <= address && address < 0x2C00))
		{
			result = ppu->nametableBankA;
		}
		else
		{
			result = ppu->nametableBankB;
		}
		break;
	case NametableMirror::HORIZONTAL_MIRROR:
		if(address < 0x2800)
		{
			result = ppu->nametableBankA;
		}
		else
		{
			result = ppu->nametableBankB;
		}
		break;
	case NametableMirror::FOUR_SCREEN_MIRROR:
		Assert(0);
		if(address < 0x2400)
		{
			result = ppu->nametableBankA;
		}
		else if(address < 0x2800)
		{
			result = ppu->nametableBankB;
		}
		else if(address < 0x2C00)
		{
			result = ppu->nametableBankC;
		}
		else
		{
			result = ppu->nametableBankD;
		}
		break;
	}

	return result;
}

static void WritePpu8(u8 byte, u16 address, Ppu *ppu, Cartridge *cartridge)
{
	if(0x2000 <= address && address < 0x3000)
	{
		u8 *nametable = GetNametableBank(address, ppu, cartridge);
		nametable[address % 0x400] = byte;
	}
}

int main()
{
	constexpr u8 windowScale = 1.0f;
	constexpr u16 nesWidth = 256;
	constexpr u16 nesHeight = 240;
	constexpr u32 windowWidth = nesWidth * windowScale;
	constexpr u32 windowHeight = nesHeight * windowScale;

	InitWindow(windowWidth, windowHeight, "tuNES");

	u8 targetFPS = 60;
	SetTargetFPS(targetFPS);

	InitialiseBackBuffers(nesWidth, nesHeight);


	Cpu *cpu = Nes::GetCpu();
	Ppu *ppu = Nes::GetPpu();
	Cartridge *cartridge = Nes::GetCartridge();

	// Ppu pixel buffer
	u32 pixelCount = nesWidth * nesHeight;
	ppu->pixelBuffer = (Color *)MemAlloc(pixelCount * sizeof(Color)); // in R8G8B8A8 format

	
	Cartridge *cartridge = Nes::GetCartridge();
	LoadCartridge("Mario Bros.nes", cartridge);

	while(!WindowShouldClose())
	{ 
		Nes::GetInstance().Update();
	}

	return 0;
}

