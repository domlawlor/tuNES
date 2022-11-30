#pragma once

#define NMI_VEC     0xFFFA
#define RESET_VEC   0xFFFC
#define IRQ_BRK_VEC 0xFFFE 
#define INSTRUCTION_COUNT 256
#define STACK_ADDRESS 0x100

#define CARRY_BIT     0x01
#define ZERO_BIT      0x02
#define INTERRUPT_BIT 0x04
#define DECIMAL_BIT   0x08
#define BREAK_BIT     0x10
#define BLANK_BIT     0x20
#define OVERFLOW_BIT  0x40
#define NEGATIVE_BIT  0x80

#define NMI_OP 0x02
#define IRQ_OP 0x12

enum class AddressMode
{
	NONE = 0, ACM, IMPL, IMED, REL,
	ZERO, ZERO_X, ZERO_Y,
	ABS, ABS_X, ABS_XW, ABS_Y, ABS_YW,
	IND, IND_X, IND_Y, IND_YW
};
typedef AddressMode AM;

constexpr u64 CpuMemorySize = Kilobytes(64);
struct Cpu
{
public:
	void InitCpu();
	void Run();

private:
	u8 ReadOpcode();
	u16 GetOperand();

	u8 ReadMemory(u16 address);
	void WriteMemory(u16 address, u8 value);

	// Direct access to memory. 
	u8 RawReadMemory(u16 address);
	void RawWriteMemory(u16 address, u8 value);

	inline void PushStack(u8 value)
	{
		WriteMemory(stackPointer + STACK_ADDRESS, value);
		--stackPointer;
	}
	inline u8 PopStack()
	{
		++stackPointer;
		return ReadMemory(stackPointer + STACK_ADDRESS);
	}

	// Status flag functions
	inline void SetCarry() { flags = flags | CARRY_BIT; }
	inline void ClearCarry() { flags = flags & ~CARRY_BIT; }
	inline void SetInterrupt() { flags = flags | INTERRUPT_BIT; }
	inline void ClearInterrupt() { flags = flags & ~INTERRUPT_BIT; }
	inline void SetDecimal() { flags = flags | DECIMAL_BIT; }
	inline void ClearDecimal() { flags = flags & ~DECIMAL_BIT; }
	inline void SetBreak() { flags = flags | BREAK_BIT; }
	inline void ClearBreak() { flags = flags & ~BREAK_BIT; }
	inline void SetBlank() { flags = flags | BLANK_BIT; }
	inline void ClearBlank() { flags = flags & ~BLANK_BIT; }
	inline void SetOverflow() { flags = flags | OVERFLOW_BIT; }
	inline void ClearOverflow() { flags = flags & ~OVERFLOW_BIT; }
	inline void SetZero(u8 value) {
		flags = (value == 0x00) ? (flags | ZERO_BIT) : (flags & ~ZERO_BIT);
	}
	inline void SetNegative(u8 value) {
		flags = (value >= 0x00 && value <= 0x7F) ? (flags & ~NEGATIVE_BIT) : (flags | NEGATIVE_BIT);
	}
	inline bool IsFlagBitSet(u8 bit) {
		return((bit & flags) != 0);
	}




private:
	u8 memory[CpuMemorySize];

	// Registers
	u8 A;
	u8 X;
	u8 Y;
	u8 flags;
	u8 stackPointer;
	u8 prgCounter;

	u16 operand;
	AddressMode addressMode;

	// Debug
	char *opName;
	u8 opCode;
};

/*
struct Cpu
{
	// TODO: Check if still needed
	bool padStrobe;

	Input inputPad1;
	u8 pad1CurrentButton;
	Input inputPad2;
	u8 pad2CurrentButton;

	u8 opLowByte;
	u8 opHighByte;
	u8 opValue;
	u8 opTemp;
};
*/