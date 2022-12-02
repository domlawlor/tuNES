#pragma once

#include "globals.h"

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

class Cartridge
{
public:
	static bool LoadCartridge(const char *fileName);

	static u8 ReadMemory(u16 address) { Assert(instance); return instance->ReadMemoryInternal(address); }
	static u8 WriteMemory(u16 address, u8 value) { Assert(instance); instance->WriteMemoryInternal(address, value); }

protected:
	
	const char *m_fileName;
	u8 *m_romFileData;
	u32 m_romFileSize;

	u8 *m_prgBanks;
	u8 *m_chrBanks;

	u8 m_prgBankCount;
	u8 m_chrBankCount;
	u8 m_prgRamSize;

	bool m_useVerticalMirror;
	bool m_hasBatteryRam;
	bool m_hasTrainer;
	bool m_useFourScreenMirror;

	u8 m_mapperNum;

	NametableMirror m_nametableMirrorType;

	// u16 m_extRegister;
	u8 m_mapperInternalReg;
	u8 m_mapperWriteCount;

	
	bool m_chr4KbMode;

private:
	virtual void Init() {};
	virtual u8 ReadMemoryInternal(u16 address) {};
	virtual void WriteMemoryInternal(u16 address, u8 value) {};

	void ClearCartridge();
	void ParseRom(const char *romFileName, u8 *romData, u32 romSize);

	static Cartridge *GetCartridgeByMapper(u8 mapperNum);

	static Cartridge *instance;
};

class NROM : public Cartridge
{
	virtual void Init() override
	{
		m_is32kPrgRom = m_prgBankCount == 2;
		Assert(m_prgBankCount <= 2);
	}
	virtual u8 ReadMemoryInternal(u16 address) override
	{
		u16 bankAddress = address - 0x8000;
		if(!m_is32kPrgRom) { bankAddress = bankAddress % 0x4000; }
		return m_prgBanks[bankAddress];
	}
	/*virtual void WriteMemoryInternal(u16 address, u8 value) override
	{

	}*/

	bool m_is32kPrgRom;
};

/*
constexpr u64 Mmc1PrgRamMaxSize = Kilobytes(32);
class MMC1 : public Cartridge
{
	virtual void Init() override
	{
		m_slot1BankNum = 0;
		m_slot2BankNum = m_prgBankCount - 1; // Filled by last bank
		m_prgRomMode = 0x3; // Initially starts here(control register is 0xC)
	}
	virtual u8 ReadMemoryInternal(u16 address) override
	{
		if(address < 0x8000)
		{

		}
		else
		{
			u16 bankAddress = address - 0x8000;
			if(!m_is32kPrgRom) { bankAddress = bankAddress % 0x4000; }
			return m_prgBanks[bankAddress];
		}
	}
	virtual void WriteMemoryInternal(u16 address, u8 value) override
	{
		

	}

	u8 prgRam[Mmc1PrgRamMaxSize];

	u8 m_slot1BankNum;
	u8 m_slot2BankNum;

	u8 m_prgRomMode;
};

class UNROM : public Cartridge
{
	virtual void Init() override
	{
		m_slot1BankNum = 0;
		m_slot2BankNum = m_prgBankCount - 1; // Filled by last bank
	}
	virtual u8 ReadMemoryInternal(u16 address) override
	{
		u16 bankAddress = address - 0x8000;
		u8 bankSlot = bankAddress / 0x4000;
		
		u8 *bank;
		// TODO
		return m_prgBanks[bankAddress];
	}
	virtual void WriteMemoryInternal(u16 address, u8 value) override
	{

	}


	u8 m_slot1BankNum;
	u8 m_slot2BankNum;
};


void AxromInit(Cartridge *cartridge, Cpu *cpu, Ppu *ppu)
{
	u8 *memoryPrgBank = cpu->memory + 0x8000;
	u8 *bankToCpy = cartridge->prgBanks + ((cartridge->prgBankCount) * Kilobytes(16)) - Kilobytes(32);
	MemoryCopy(memoryPrgBank, bankToCpy, Kilobytes(32));

	//ppu->mirrorType = SINGLE_SCREEN_BANK_A;
	cartridge->nametableMirrorType = NametableMirror::SINGLE_SCREEN_BANK_A;
}

void Mmc1Update(Nes *nes, u8 byteWritten, u16 address)
{
	Cpu *cpu = Nes::GetCpu();
	Ppu *ppu = Nes::GetPpu();
	Cartridge *cartridge = Nes::GetCartridge();

	u16 prgRomBank1 = 0x8000;
	u16 prgRomBank2 = 0xC000;

	bool isLargePrg = (cartridge->prgBankCount > 16);
	bool isLargeChr = (cartridge->chrBankCount > 1);


	bool isClearBitSet = (byteWritten & (1 << 7)) != 0;
	if(isClearBitSet)
	{
		cartridge->mapperWriteCount = 0;
		byteWritten = 0;
		cartridge->mapperInternalReg = 0;
	}
	else
	{
		++cartridge->mapperWriteCount;

		// Shifted where the first write is the least significant, last write is most significant
		cartridge->mapperInternalReg = (cartridge->mapperInternalReg >> 1);
		cartridge->mapperInternalReg = (cartridge->mapperInternalReg & 0xF); // clear 5-bit
		cartridge->mapperInternalReg |= ((byteWritten & 1) << 4);

		if(cartridge->mapperWriteCount == 5) // On 5th write
		{
			u8 dataReg = cartridge->mapperInternalReg;

			// TODO: Potentially change all address bounds checks to this style?
			bool bit13Set = (address & (1 << 13)) != 0;
			bool bit14Set = (address & (1 << 14)) != 0;

			if(!bit13Set && !bit14Set)     // Control Reg
			{
				u8 mirror = dataReg & 3;
				if(mirror == 0)
					m_nametableMirrorType = NametableMirror::SINGLE_SCREEN_BANK_A;
				if(mirror == 1)
					m_nametableMirrorType = NametableMirror::SINGLE_SCREEN_BANK_B;
				if(mirror == 2)
					m_nametableMirrorType = NametableMirror::VERTICAL_MIRROR;
				if(mirror == 3)
					cartridge->nametableMirrorType = NametableMirror::HORIZONTAL_MIRROR;

				u8 prevPrgRomMode = cartridge->prgRomMode;
				cartridge->prgRomMode = (dataReg & 0xC) >> 2;

				if(prevPrgRomMode != cartridge->prgRomMode) // TODO: Update the banks
				{
					if(cartridge->prgRomMode == 2) // 16kb fixed low bank
					{
						u8 *bankToCpy = cartridge->prgBanks;
						MemoryCopy((u8 *)cpu->memory + 0x8000, bankToCpy, Kilobytes(16));
					}
					else if(cartridge->prgRomMode == 3) // 16kb fixed high bank, use last bank
					{
						u8 *bankToCpy = cartridge->prgBanks + ((cartridge->prgBankCount - 1) * Kilobytes(16));
						MemoryCopy((u8 *)cpu->memory + 0xC000, bankToCpy, Kilobytes(16));
					}
				}

				cartridge->chr4KbMode = ((dataReg & 0x10) == 0);
			}
			else if(bit13Set && !bit14Set) // CHR Bank 0
			{
				u8 sizeToCpy = 0;

				if(cartridge->chr4KbMode)
				{
					sizeToCpy = (u8)Kilobytes(4);
				}
				else
				{
					dataReg = dataReg >> 1; // 8kb mode Low bit ignored
					sizeToCpy = (u8)Kilobytes(8);
				}

				// TODO: We shouldn't need to copy the memory to another place.
				//       Bank switch is a direct line to memory, so it should just change a pointer

				// If CHR bank count is 0, then it uses RAM?? TODO: Check
				if(cartridge->chrBankCount > 0) {
					u8 *bankToCpy = cartridge->chrBanks + (dataReg * sizeToCpy);
					MemoryCopy((u8 *)ppu->memory, bankToCpy, sizeToCpy);
				}
			}
			else if(!bit13Set && bit14Set) // CHR Bank 1
			{
				if(cartridge->chr4KbMode)
				{
					// TODO: mmc1 variants write other bits. Investigate

					if(cartridge->chrBankCount > 0)
					{
						u8 *bankToCpy = cartridge->chrBanks + (dataReg * Kilobytes(4));
						MemoryCopy(ppu->memory + 0x1000, bankToCpy, Kilobytes(4));
					}
				}
			}
			else if(bit13Set && bit14Set) // PRG bank
			{
				dataReg &= 0xF; // mask away MSB/5th bit, not used for bank num
				Assert(dataReg < cartridge->prgBankCount);

				if(cartridge->prgRomMode == 0 || cartridge->prgRomMode == 1) // 32kib Mode
				{
					dataReg = dataReg >> 1;
					u8 *bankToCpy = cartridge->prgBanks + (dataReg * Kilobytes(32));
					MemoryCopy(cpu->memory + 0x8000, bankToCpy, Kilobytes(32));
				}
				else if(cartridge->prgRomMode == 2) // 16kb fixed low bank - swap high bank
				{
					u8 *bankToCpy = cartridge->prgBanks + (dataReg * Kilobytes(16));
					MemoryCopy(cpu->memory + 0xC000, bankToCpy, Kilobytes(16));
				}
				else if(cartridge->prgRomMode == 3) // 16kb fixed high bank - swap low bank
				{
					u8 *bankToCpy = cartridge->prgBanks + (dataReg * Kilobytes(16));
					MemoryCopy(cpu->memory + 0x8000, bankToCpy, Kilobytes(16));
				}
			}

			cartridge->mapperWriteCount = 0;
			cartridge->mapperInternalReg = 0;
		}
	}
}

void UnromUpdate(Nes *nes, u8 byteWritten, u16 address)
{
	Cpu *cpu = Nes::GetCpu();
	Ppu *ppu = Nes::GetPpu();
	Cartridge *cartridge = Nes::GetCartridge();

	u8 *memPrgBank1 = cpu->memory + 0x8000;

	const u8 bankNumber = byteWritten;
	u8 *bankToCpy = cartridge->prgBanks + (bankNumber * Kilobytes(16));

	MemoryCopy(memPrgBank1, bankToCpy, Kilobytes(16));
}

void AxromUpdate(Nes *nes, u8 byteWritten, u16 address)
{
	Cpu *cpu = Nes::GetCpu();
	Ppu *ppu = Nes::GetPpu();
	Cartridge *cartridge = Nes::GetCartridge();

	u8 *memoryPrgBank = cpu->memory + 0x8000;

	const u8 selectedBank = byteWritten & 7;
	u8 *bankToCpy = cartridge->prgBanks + (selectedBank * Kilobytes(32));

	MemoryCopy(memoryPrgBank, bankToCpy, Kilobytes(32));

	// Nametable Single Screen bank select
	if(byteWritten & 0x10)
	{
		cartridge->nametableMirrorType = NametableMirror::SINGLE_SCREEN_BANK_B;
	}
	else
	{
		cartridge->nametableMirrorType = NametableMirror::SINGLE_SCREEN_BANK_A;
	}
}
*/

Cartridge *Cartridge::GetCartridgeByMapper(u8 mapperNumber)
{
	switch(mapperNumber)
	{
	case 0: return new NROM(); break;
	//case 1: return new MMC1();
	default: TraceLog(LOG_ERROR, "Missing mapper class!!"); return nullptr; break;
	};

}