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

enum AddressType
{
	ACM = 0, IMPL, IMED, REL,
	ZERO_R, ZERO_RW, ZERO_W,
	ZERX_R, ZERX_RW, ZERX_W,
	ZERY_R, ZERY_RW, ZERY_W,
	ABS_R, ABS_RW, ABS_W,
	ABSX_R, ABSX_RW, ABSX_W,
	ABSY_R, ABSY_RW, ABSY_W,
	INDX_R, INDX_RW, INDX_W,
	INDY_R, INDY_RW, INDY_W,
	ABSJ, INDI
};

constexpr u64 CpuMemorySize = Kilobytes(64);

struct Cpu
{
	u8 memory[CpuMemorySize];

	u8 A;
	u8 X;
	u8 Y;
	u8 flags;
	u8 stackPtr;
	u16 prgCounter;
	
	// TODO: Check if still needed
	bool padStrobe;

	Input inputPad1;
	u8 pad1CurrentButton;
	Input inputPad2;
	u8 pad2CurrentButton;

	char *opName;
	u8 opCode;
	u8 opClockTotal;
	u8 addressType;
	u8 opLowByte;
	u8 opHighByte;
	u8 opValue;
	u8 opTemp;

	// Timing
	u16 catchupClocks;
	u16 lastClocksIntoOp;

#if CPU_LOG
	// TODO: Make platform independant. Hold the pointer?
	u8 logA;
	u8 logX;
	u8 logY;
	u8 logSP;
	u8 logFlags;
	u16 logPC;
	u8 logOp;
	char *logOpName;
#endif
};