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
	static Cartridge *CreateCartridgeForRom(u8 *romData, u32 romSize);
	
	~Cartridge() { UnloadFileData(m_romFileData); }

	virtual void Init() = 0;
	virtual u8 ReadMemory(u16 address) = 0;
	virtual void WriteMemory(u16 address, u8 value) = 0;

	virtual u8 ReadCHRMemory(u16 address) = 0;
	virtual void WriteCHRMemory(u16 address, u8 value) = 0;
	
	NametableMirror GetNametableMirrorType() { return m_nametableMirrorType; }
protected:
	Cartridge() {};

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
private:
	void ParseRom(u8 *romData, u32 romSize);
};


constexpr u64 NROMChrMemorySize = Kilobytes(8);

class NROM : public Cartridge
{
public:
	virtual void Init() override
	{
		m_is32kPrgRom = m_prgBankCount == 2;
		Assert(m_prgBankCount <= 2);
	}
	virtual u8 ReadMemory(u16 address) override
	{
		u16 bankAddress = address - 0x8000;
		if(!m_is32kPrgRom) { bankAddress = bankAddress % 0x4000; }
		u8 val = m_prgBanks[bankAddress];
		return val;
	}
	virtual void WriteMemory(u16 address, u8 value) override
	{

	}

	virtual u8 ReadCHRMemory(u16 address) override
	{
		u8 value = m_chrBanks[address];
		return value;
	}
	virtual void WriteCHRMemory(u16 address, u8 value) override { }

private:

	bool m_is32kPrgRom = false;
};


// use the smallest bank size and calc in functions if using bigger bank mode
constexpr u64 Mmc1PrgBankSize = 0x4000;
constexpr u64 Mmc1ChrBankSize = 0x1000;
constexpr u64 Mmc1PrgRamMaxSize = Kilobytes(32);
constexpr u64 Mmc1ChrRamMaxSize = Kilobytes(8);
class MMC1 : public Cartridge
{
public:
	virtual void Init() override
	{
		// init state
		// first bank on cartridge mapped to 0x8000
		// last bank on cartridge mapped to 0xc000
		MemorySet(m_prgRam, 0, Mmc1PrgRamMaxSize);
		MemorySet(m_chrRam, 0, Mmc1ChrRamMaxSize);

		m_prgBankNumSlotA = 0;
		m_prgBankNumSlotB = m_prgBankCount - 1; // Filled by last bank
		m_controlReg = 0x0C; // Initially starts here(control register is 0xC)
		ApplyUpdates();
	}

	virtual u8 ReadMemory(u16 address) override
	{
		if(address < 0x8000) // save/work ram
		{
			u8 value = m_prgRam[address % Mmc1PrgRamMaxSize];
			return value;
		}
		else
		{
			u32 bankNum = (address >= 0xC000) ? m_prgBankNumSlotB : m_prgBankNumSlotA;
			u8 *bank = m_prgBanks + (bankNum * Mmc1PrgBankSize);
			u8 value = bank[address % Mmc1PrgBankSize];
			return value;
		}
	}
	virtual void WriteMemory(u16 address, u8 value) override
	{
		if(address < 0x8000) // save/work ram
		{
			m_prgRam[address % Mmc1PrgRamMaxSize] = value;
		}
		else
		{
			bool isClearBitSet = (value & 0x80) != 0;
			if(isClearBitSet)
			{
				m_writeReg = 0;
				m_writeCount = 0;

				m_controlReg = m_controlReg |= 0x0C;
				ApplyUpdates();
			}
			else
			{
				// Shifted where the first write is the least significant, last write is most significant
				m_writeReg = (m_writeReg >> 1);
				m_writeReg |= ((value << 4) & 0x10);

				++m_writeCount;

				if(m_writeCount == 5) // On 5th write
				{
					u16 registerNum = (address & 0x6000) >> 13;

					if(registerNum == 0)
					{
						m_controlReg = m_writeReg;
					}
					else if(registerNum == 1)
					{
						m_chrBankNumSlotA = m_writeReg;
					}
					else if(registerNum == 2)
					{
						m_chrBankNumSlotB = m_writeReg;
					}
					else if(registerNum == 3)
					{
						m_prgBankReg = m_writeReg;
					}

					ApplyUpdates();

					m_writeReg = 0;
					m_writeCount = 0;
				}
			}
		}
	}

	virtual u8 ReadCHRMemory(u16 address) override
	{
		if(m_chrBankCount == 0)
		{
			u8 value = m_chrRam[address % Mmc1ChrRamMaxSize];
			return value;
		}
		else
		{
			u32 bankNum = (address >= 0x1000) ? m_chrBankNumSlotB : m_chrBankNumSlotA;
			u8 *bank = m_chrBanks + (bankNum * Mmc1ChrBankSize);
			u8 value = bank[address % Mmc1ChrBankSize];
			return value;
		}
	}
	virtual void WriteCHRMemory(u16 address, u8 value) override
	{
		if(m_chrBankCount == 0)
		{
			m_chrRam[address % Mmc1ChrRamMaxSize] = value;
		}
		else
		{
			Assert(0);
			/*u32 bankNum = (address >= 0x1000) ? m_chrBankNumSlotB : m_chrBankNumSlotA;
			u8 *bank = m_chrBanks + (bankNum * Mmc1ChrBankSize);
			bank[address % Mmc1ChrBankSize] = value;*/
		}
	}

private:
	void ApplyUpdates()
	{
		// CHR Bank
		bool inChrBank4KbMode = ((m_controlReg & 0x10) != 0);
	
		u32 chrBankNumA = m_chrBankRegA & 0x1F;
		u32 chrBankNumB = m_chrBankRegB & 0x1F;

		if(inChrBank4KbMode)
		{
			m_chrBankNumSlotA = chrBankNumA;
			m_chrBankNumSlotB = chrBankNumB;
		}
		else // 8kb chr banks
		{
			u32 chrBank8kbNum = chrBankNumA & 0x1E; // Ignore bottom bit
			m_chrBankNumSlotA = chrBank8kbNum;
			m_chrBankNumSlotB = chrBank8kbNum + 1;
		}

		// PRG bank
		bool prgBankSwapLowSlot = (m_controlReg & 0x04) != 0;
		bool prgBankModeIs16k = (m_controlReg & 0x08) != 0;

		u32 prgBankNum = m_prgBankReg & 0xF;

		if(prgBankModeIs16k)
		{
			if(prgBankSwapLowSlot)
			{
				m_prgBankNumSlotA = prgBankNum;
				m_prgBankNumSlotB = m_prgBankCount - 1;
			}
			else
			{
				m_prgBankNumSlotA = 0;
				m_prgBankNumSlotB = prgBankNum;
			}
		}
		else // 32k prg banks
		{
			u32 prgBank32kbNum = prgBankNum & 0xFE; // Ignore bottom bit
			m_prgBankNumSlotA = prgBank32kbNum;
			m_prgBankNumSlotB = prgBank32kbNum + 1;
		}

		u8 mirrorType = m_controlReg & 0x03;
		if(mirrorType == 0) { m_nametableMirrorType = NametableMirror::SINGLE_SCREEN_BANK_A; }
		if(mirrorType == 1) { m_nametableMirrorType = NametableMirror::SINGLE_SCREEN_BANK_B; }
		if(mirrorType == 2) { m_nametableMirrorType = NametableMirror::VERTICAL_MIRROR; }
		if(mirrorType == 3) { m_nametableMirrorType = NametableMirror::HORIZONTAL_MIRROR; }
	}

	u8 m_prgRam[Mmc1PrgRamMaxSize];
	u8 m_chrRam[Mmc1ChrRamMaxSize];
	

	u32 m_prgBankNumSlotA = 0;
	u32 m_prgBankNumSlotB = 0;

	u32 m_chrBankNumSlotA = 0;
	u32 m_chrBankNumSlotB = 0;

	u8 m_controlReg = 0;
	u8 m_chrBankRegA = 0;
	u8 m_chrBankRegB = 0;
	u8 m_prgBankReg = 0;

	u8 m_writeReg = 0;
	u8 m_writeCount = 0;
};


constexpr u32 unromPrgBankSize = 0x4000;
constexpr u32 unromChrRAMSize = 0x2000;

class UNROM : public Cartridge
{
public:
	virtual void Init() override
	{
		m_slot1BankNum = 0;
	}

	virtual u8 ReadMemory(u16 address) override
	{
		u32 bankNum = (address >= 0xC000) ? (m_prgBankCount - 1) : m_slot1BankNum;
		u8 *bank = m_prgBanks + (bankNum * unromPrgBankSize);
		u8 value = bank[address % unromPrgBankSize];
		return value;
	}
	virtual void WriteMemory(u16 address, u8 value) override
	{
		m_slot1BankNum = value & 0x7;
	}

	virtual u8 ReadCHRMemory(u16 address) override
	{
		u8 value = m_chrRam[address % unromChrRAMSize];
		return value;
	}
	virtual void WriteCHRMemory(u16 address, u8 value) override
	{ 
		m_chrRam[address % unromChrRAMSize] = value;
	}

private:
	u8 m_chrRam[unromChrRAMSize];
	u8 m_slot1BankNum = 0;
};


constexpr u32 axromPrgBankSize = 0x8000; // 32kb
constexpr u32 axromChrRAMSize = 0x2000; // 8kb

class AXROM : public Cartridge
{
public:
	virtual void Init() override
	{
		u32 bank32kbCount = m_prgBankCount / 2;

		m_bankNum = bank32kbCount - 1;
		m_nametableMirrorType = NametableMirror::SINGLE_SCREEN_BANK_A;
	}

	virtual u8 ReadMemory(u16 address) override
	{
		u8 *bank = m_prgBanks + (m_bankNum * axromPrgBankSize);
		u8 value = bank[address % axromPrgBankSize];
		return value;
	}
	virtual void WriteMemory(u16 address, u8 value) override
	{
		m_bankNum = value & 0x7;
		m_nametableMirrorType = ((value & 0x10) != 0) ? NametableMirror::SINGLE_SCREEN_BANK_B : NametableMirror::SINGLE_SCREEN_BANK_A;
	}

	virtual u8 ReadCHRMemory(u16 address) override
	{
		u8 value = m_chrRam[address % axromChrRAMSize];
		return value;
	}
	virtual void WriteCHRMemory(u16 address, u8 value) override
	{
		m_chrRam[address % axromChrRAMSize] = value;
	}

private:
	u8 m_chrRam[axromChrRAMSize];
	u8 m_bankNum = 0;
};

constexpr u32 cnromPrgBankSize = 0x8000; // 32kb
constexpr u32 cnromChrRAMSize = 0x2000; // 8kb

class CNROM : public Cartridge
{
public:
	virtual void Init() override
	{
		m_chrBankNum = 0;
	}

	virtual u8 ReadMemory(u16 address) override
	{
		u8 value = m_prgBanks[address % 0x8000];
		return value;
	}
	virtual void WriteMemory(u16 address, u8 value) override
	{
		m_chrBankNum = value;
	}

	virtual u8 ReadCHRMemory(u16 address) override
	{
		u8 *chrBank = m_chrBanks + (m_chrBankNum * cnromChrRAMSize);
		u8 value = chrBank[address % cnromChrRAMSize];
		return value;
	}
	virtual void WriteCHRMemory(u16 address, u8 value) override
	{
	}

private:
	u32 m_chrBankNum;
};