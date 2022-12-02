#pragma once

#define NMI_VEC     0xFFFA
#define RESET_VEC   0xFFFC
#define IRQ_BRK_VEC 0xFFFE 
#define STACK_ADDRESS 0x100
#define INSTRUCTION_COUNT 256

#define CARRY_BIT     0x01
#define ZERO_BIT      0x02
#define INTERRUPT_BIT 0x04
#define DECIMAL_BIT   0x08
#define BREAK_BIT     0x10
#define BLANK_BIT     0x20
#define OVERFLOW_BIT  0x40
#define NEGATIVE_BIT  0x80

//#define NMI_OP 0x02
//#define IRQ_OP 0x12

enum class AddressMode
{
	NONE = 0, ACM, IMPL, IMED, REL,
	ZERO, ZERO_X, ZERO_Y,
	ABS, ABS_X, ABS_XW, ABS_Y, ABS_YW,
	IND, IND_X, IND_Y, IND_YW
};
typedef AddressMode AM;

static AddressMode OpAddressModes[INSTRUCTION_COUNT] =
{
	/*          0         1           2         3           4           5           6           7           8         9           A         B           C          D          E           F  */
	/*0*/   AM::IMPL, AM::IND_X,  AM::NONE, AM::IND_X,  AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::IMPL, AM::IMED,   AM::ACM,  AM::IMED,   AM::ABS,   AM::ABS,   AM::ABS,    AM::ABS,
	/*1*/   AM::REL,  AM::IND_Y,  AM::NONE, AM::IND_YW, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::IMPL, AM::ABS_Y,  AM::IMPL, AM::ABS_YW, AM::ABS_X, AM::ABS_X, AM::ABS_XW, AM::ABS_XW,
	/*2*/   AM::IMPL, AM::IND_X,  AM::NONE, AM::IND_X,  AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::IMPL, AM::IMED,   AM::ACM,  AM::IMED,   AM::ABS,   AM::ABS,   AM::ABS,    AM::ABS,
	/*3*/   AM::REL,  AM::IND_Y,  AM::NONE, AM::IND_YW, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::IMPL, AM::ABS_Y,  AM::IMPL, AM::ABS_YW, AM::ABS_X, AM::ABS_X, AM::ABS_XW, AM::ABS_XW,
	/*4*/   AM::IMPL, AM::IND_X,  AM::NONE, AM::IND_X,  AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::IMPL, AM::IMED,   AM::ACM,  AM::IMED,   AM::ABS,   AM::ABS,   AM::ABS,    AM::ABS,
	/*5*/   AM::REL,  AM::IND_Y,  AM::NONE, AM::IND_YW, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::IMPL, AM::ABS_Y,  AM::IMPL, AM::ABS_YW, AM::ABS_X, AM::ABS_X, AM::ABS_XW, AM::ABS_XW,
	/*6*/   AM::IMPL, AM::IND_X,  AM::NONE, AM::IND_X,  AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::IMPL, AM::IMED,   AM::ACM,  AM::IMED,   AM::IND,   AM::ABS,   AM::ABS,    AM::ABS,
	/*7*/   AM::REL,  AM::IND_Y,  AM::NONE, AM::IND_YW, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::IMPL, AM::ABS_Y,  AM::IMPL, AM::ABS_YW, AM::ABS_X, AM::ABS_X, AM::ABS_XW, AM::ABS_XW,
	/*8*/   AM::IMED, AM::IND_X,  AM::IMED, AM::IND_X,  AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::IMPL, AM::IMED,   AM::IMPL, AM::IMED,   AM::ABS,   AM::ABS,   AM::ABS,    AM::ABS,
	/*9*/   AM::REL,  AM::IND_YW, AM::NONE, AM::IND_YW, AM::ZERO_X, AM::ZERO_X, AM::ZERO_Y, AM::ZERO_Y, AM::IMPL, AM::ABS_YW, AM::IMPL, AM::ABS_YW, AM::ABS_X, AM::ABS_X, AM::ABS_YW, AM::ABS_YW,
	/*A*/   AM::IMED, AM::IND_X,  AM::IMED, AM::IND_X,  AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::IMPL, AM::IMED,   AM::IMPL, AM::IMED,   AM::ABS,   AM::ABS,   AM::ABS,    AM::ABS,
	/*B*/   AM::REL,  AM::IND_Y,  AM::NONE, AM::IND_Y,  AM::ZERO_X, AM::ZERO_X, AM::ZERO_Y, AM::ZERO_Y, AM::IMPL, AM::ABS_Y,  AM::IMPL, AM::ABS_Y,  AM::ABS_X, AM::ABS_X, AM::ABS_Y,  AM::ABS_Y,
	/*C*/   AM::IMED, AM::IND_X,  AM::IMED, AM::IND_X,  AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::IMPL, AM::IMED,   AM::IMPL, AM::IMED,   AM::ABS,   AM::ABS,   AM::ABS,    AM::ABS,
	/*D*/   AM::REL,  AM::IND_Y,  AM::NONE, AM::IND_YW, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::IMPL, AM::ABS_Y,  AM::IMPL, AM::ABS_YW, AM::ABS_X, AM::ABS_X, AM::ABS_XW, AM::ABS_XW,
	/*E*/   AM::IMED, AM::IND_X,  AM::IMED, AM::IND_X,  AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::ZERO,   AM::IMPL, AM::IMED,   AM::IMPL, AM::IMED,   AM::ABS,   AM::ABS,   AM::ABS,    AM::ABS,
	/*F*/   AM::REL,  AM::IND_Y,  AM::NONE, AM::IND_YW, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::ZERO_X, AM::IMPL, AM::ABS_Y,  AM::IMPL, AM::ABS_YW, AM::ABS_X, AM::ABS_X, AM::ABS_XW, AM::ABS_XW,
};

static char *OpNames[INSTRUCTION_COUNT] =
{
	/*         0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F        */
	/*0*/  "BRK","ORA","NMI","SLO","SKB","ORA","ASL","SLO","PHP","ORA","ASL","ANC","SKW","ORA","ASL","SLO",
	/*1*/  "BPL","ORA","IRQ","SLO","SKB","ORA","ASL","SLO","CLC","ORA","NOP","SLO","SKW","ORA","ASL","SLO",
	/*2*/  "JSR","AND","KIL","RLA","BIT","AND","ROL","RLA","PLP","AND","ROL","ANC","BIT","AND","ROL","RLA",
	/*3*/  "BMI","AND","KIL","RLA","SKB","AND","ROL","RLA","SEC","AND","NOP","RLA","SKW","AND","ROL","RLA",
	/*4*/  "RTI","EOR","KIL","SRE","SKB","EOR","LSR","SRE","PHA","EOR","LSR","ALR","JMP","EOR","LSR","SRE",
	/*5*/  "BVC","EOR","KIL","SRE","SKB","EOR","LSR","SRE","CLI","EOR","NOP","SRE","SKW","EOR","LSR","SRE",
	/*6*/  "RTS","ADC","KIL","RRA","SKB","ADC","ROR","RRA","PLA","ADC","ROR","ARR","JMP","ADC","ROR","RRA",
	/*7*/  "BVS","ADC","KIL","RRA","SKB","ADC","ROR","RRA","SEI","ADC","NOP","RRA","SKW","ADC","ROR","RRA",
	/*8*/  "SKB","STA","SKB","SAX","STY","STA","STX","SAX","DEY","SKB","TXA","XAA","STY","STA","STX","SAX",
	/*9*/  "BCC","STA","KIL","AHX","STY","STA","STX","SAX","TYA","STA","TXS","TAS","SHY","STA","SHX","AHX",
	/*A*/  "LDY","LDA","LDX","LAX","LDY","LDA","LDX","LAX","TAY","LDA","TAX","LAX","LDY","LDA","LDX","LAX",
	/*B*/  "BCS","LDA","KIL","LAX","LDY","LDA","LDX","LAX","CLV","LDA","TSX","LAS","LDY","LDA","LDX","LAX",
	/*C*/  "CPY","CMP","SKB","DCP","CPY","CMP","DEC","DCP","INY","CMP","DEX","AXS","CPY","CMP","DEC","DCP",
	/*D*/  "BNE","CMP","KIL","DCP","SKB","CMP","DEC","DCP","CLD","CMP","NOP","DCP","SKW","CMP","DEC","DCP",
	/*E*/  "CPX","SBC","SKB","ISC","CPX","SBC","INC","ISC","INX","SBC","NOP","SBC","CPX","SBC","INC","ISC",
	/*F*/  "BEQ","SBC","KIL","ISC","SKB","SBC","INC","ISC","SED","SBC","NOP","ISC","SKW","SBC","INC","ISC"
};

constexpr u64 CpuMemorySize = Kilobytes(64);

class Cpu
{
	u8 memory[CpuMemorySize];

	// Registers
	u8 A;
	u8 X;
	u8 Y;
	u8 flags;
	u8 stackPointer;
	u16 prgCounter;

	u16 operand;
	AddressMode addressMode;

	// Debug
	char *opName;
	u8 opCode;

	u64 masterClock;

	u8 clockCyclesPreCatchup;
	u8 clockCyclesPostCatchup;



	// TODO: Check if still needed
	/*
	bool padStrobe;

	Input inputPad1;
	u8 pad1CurrentButton;
	Input inputPad2;
	u8 pad2CurrentButton;
	*/

	u8 irqFlag;
	u8 activeIrqs;

	bool triggerIrq;
	bool prevTriggerIrq;

	bool nmiSet; // Set from ppu
	bool triggerNmi;
	bool prevTriggerNmi;

public:
	void Init();
	void OnReset();

	void Run();

private:
	u16 ReadOperand();

	u8 ReadMemory(u16 address);
	void WriteMemory(u16 address, u8 value);

	// Direct access to memory. 
	u8 RawReadMemory(u16 address);
	void RawWriteMemory(u16 address, u8 value);

	void UpdateInterrupts();

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
		flags = (value & 0x80) ? (flags | NEGATIVE_BIT) : (flags & ~NEGATIVE_BIT);
	}
	inline bool IsFlagBitSet(u8 bit) { return((bit & flags) != 0); }

	inline bool IsCrossPageBoundary(u16 addressA, u16 addressB)
	{
		return (addressA & 0xFF00) != (addressB & 0xFF00);
	}

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

	u8 GetOpValue()
	{
		if(addressMode < AddressMode::ZERO) { return (u8)operand; }
		else { return ReadMemory(operand); }
	}

	void NMI();
	void IRQ();

	// Operations
	void SEI() { SetInterrupt(); }
	void SEC() { SetCarry(); }
	void SED() { SetDecimal(); }

	void CLI() { ClearInterrupt(); }
	void CLC() { ClearCarry(); }
	void CLD() { ClearDecimal(); }
	void CLV() { ClearOverflow(); }

	void EOR()
	{
		A = A ^ GetOpValue();
		SetZero(A);
		SetNegative(A);
	}
	void AND()
	{
		A = A & GetOpValue();
		SetZero(A);
		SetNegative(A);
	}
	void ORA()
	{
		A = A | GetOpValue();
		SetZero(A);
		SetNegative(A);
	}

	void LDA() 
	{ 
		A = GetOpValue();
		SetZero(A);
		SetNegative(A);
	}
	void LDX()
	{
		X = GetOpValue();
		SetZero(X);
		SetNegative(X);
	}
	void LDY()
	{
		Y = GetOpValue();
		SetZero(Y);
		SetNegative(Y);
	}
	void STA() { WriteMemory(operand, A); }
	void STX() { WriteMemory(operand, X); }
	void STY() { WriteMemory(operand, Y); }

	void PHA() { PushStack(A); }
	void PHP()
	{
		SetBreak();
		SetBlank();
		PushStack(flags);
	}
	void PLA()
	{
		ReadMemory(prgCounter); // dummy read
		A = PopStack();
		SetZero(A);
		SetNegative(A);
	}
	void PLP()
	{
		ReadMemory(prgCounter); // dummy read
		u8 value = PopStack();
		flags = value & 0xCF;
	}

	void INC()
	{
		u8 value = ReadMemory(operand);
		WriteMemory(operand, value); // Dummy
		value += 1;
		SetZero(value);
		SetNegative(value);
		WriteMemory(operand, value);
	}
	void DEC()
	{
		u8 value = ReadMemory(operand);
		WriteMemory(operand, value); // Dummy
		value -= 1;
		SetZero(value);
		SetNegative(value);
		WriteMemory(operand, value);
	}

	void DEX()
	{
		--X;
		SetZero(X);
		SetNegative(X);
	}
	void INX()
	{
		++X;
		SetZero(X);
		SetNegative(X);
	}
	void DEY()
	{
		--Y;
		SetZero(Y);
		SetNegative(Y);
	}
	void INY()
	{
		++Y;
		SetZero(Y);
		SetNegative(Y);
	}

	void TAX()
	{
		X = A;
		SetZero(X);
		SetNegative(X);
	}
	void TXA()
	{
		A = X;
		SetZero(A);
		SetNegative(A);
	}
	void TAY()
	{
		Y = A;
		SetZero(Y);
		SetNegative(Y);
	}
	void TYA()
	{
		A = Y;
		SetZero(A);
		SetNegative(A);
	}
	void TSX()
	{
		X = stackPointer;
		SetZero(X);
		SetNegative(X);
	}
	void TXS() { stackPointer = X; }

	void ASL()
	{
		if(addressMode == AddressMode::ACM)
		{
			if(A & 0x80) { SetCarry(); } else { ClearCarry(); }
			A = A << 1;
			SetZero(A);
			SetNegative(A);
		}
		else
		{
			u8 value = ReadMemory(operand);
			WriteMemory(operand, value); // Dummy write
			//ASL Op here

			if(value & 0x80) { SetCarry(); } else { ClearCarry(); }
			value = value << 1;
			SetZero(value);
			SetNegative(value);
			WriteMemory(operand, value);
		}
	}
	void LSR()
	{
		if(addressMode == AddressMode::ACM)
		{
			if(A & 0x01) { SetCarry(); } else { ClearCarry(); }
			A = A >> 1;
			SetZero(A);
			SetNegative(A);
		} else
		{
			u8 value = ReadMemory(operand);
			WriteMemory(operand, value); // Dummy write
			//ASL Op here

			if(value & 0x01) { SetCarry(); } else { ClearCarry(); }
			value = value >> 1;
			SetZero(value);
			SetNegative(value);
			WriteMemory(operand, value);
		}
	}
	void ROL()
	{
		bool isCarrySet = IsFlagBitSet(CARRY_BIT);

		if(addressMode == AddressMode::ACM)
		{
			if(A & 0x80) { SetCarry(); } else { ClearCarry(); }
			A = A << 1;
			if(isCarrySet) { A = A | 0x01; }
			SetZero(A);
			SetNegative(A);
		} else
		{
			u8 value = ReadMemory(operand);
			WriteMemory(operand, value); // Dummy write
			//ASL Op here

			if(value & 0x80) { SetCarry(); } else { ClearCarry(); }
			value = value << 1;
			if(isCarrySet) { value = value | 0x01; }
			SetZero(value);
			SetNegative(value);
			WriteMemory(operand, value);
		}
	}
	void ROR()
	{
		bool isCarrySet = IsFlagBitSet(CARRY_BIT);

		if(addressMode == AddressMode::ACM)
		{
			if(A & 0x01) { SetCarry(); } else { ClearCarry(); }
			A = A >> 1;
			if(isCarrySet) { A = A | 0x80; }
			SetZero(A);
			SetNegative(A);
		} else
		{
			u8 value = ReadMemory(operand);
			WriteMemory(operand, value); // Dummy write
			//ASL Op here

			if(value & 0x01) { SetCarry(); } else { ClearCarry(); }
			value = value >> 1;
			if(isCarrySet) { value = value | 0x80; }
			SetZero(value);
			SetNegative(value);
			WriteMemory(operand, value);
		}
	}


	void NOP() { GetOpValue(); /* Sort of dummy read for cycles */ }

	void AddOperation(u8 valueToAdd)
	{
		u16 sum = A + valueToAdd;
		if(IsFlagBitSet(CARRY_BIT))
		{
			sum += 1;
		}

		if(sum > 0xFF) { SetCarry(); }
		else { ClearCarry(); }
		if(~(A ^ valueToAdd) & (A ^ sum) & 0x80) { SetOverflow(); }
		else { ClearOverflow(); }
		A = sum;

		SetZero(A);
		SetNegative(A);
	}

	void ADC()
	{
		u8 opValue = GetOpValue();
		AddOperation(opValue);
	}
	void SBC()
	{
		u8 opValue = GetOpValue() ^ 0xFF;
		AddOperation(opValue);
	}

	void CMP()
	{
		u8 opValue = GetOpValue();
		u16 cmpValue = A - opValue;
		SetZero(cmpValue);
		SetNegative(cmpValue);
		if(A >= opValue) { SetCarry(); } else { ClearCarry(); }
	}
	void CPX()
	{
		u8 opValue = GetOpValue();
		u16 cmpValue = X - opValue;
		SetZero(cmpValue);
		SetNegative(cmpValue);
		if(X >= opValue) { SetCarry(); } else { ClearCarry(); }
	}
	void CPY()
	{
		u8 opValue = GetOpValue();
		u16 cmpValue = Y - opValue;
		SetZero(cmpValue);
		SetNegative(cmpValue);
		if(Y >= opValue) { SetCarry(); } else { ClearCarry(); }
	}

	void BIT()
	{
		u8 opValue = GetOpValue();
		SetZero(A & opValue);
		SetNegative(opValue);
		if(opValue & 0x40) { SetOverflow(); } else { ClearOverflow(); }
	}

	void BRK()
	{
		u16 addressToPush = prgCounter + 1;
		u8 highByte = (addressToPush >> 8) & 0xFF;
		u8 lowByte = addressToPush & 0xFF;
		PushStack(highByte);
		PushStack(lowByte);

		u8 brkFlags = flags | BREAK_BIT | BLANK_BIT;
		PushStack(brkFlags);

		SetInterrupt();

		bool nmiTriggered = false; // TODO: nmiTriggered
		if(nmiTriggered)
		{
			prgCounter = (ReadMemory(NMI_VEC+1) << 8) | ReadMemory(NMI_VEC);
		}
		else
		{
			prgCounter = (ReadMemory(IRQ_BRK_VEC + 1) << 8) | ReadMemory(IRQ_BRK_VEC);
		}
		// prevNmiTrigged = false;
	}

	void JMP()
	{
		if(addressMode == AM::ABS)
		{
			prgCounter = operand;
		}
		else
		{
			Assert(addressMode == AM::IND);
			u16 indirectAddress = operand;
			if((indirectAddress & 0xFF) == 0xFF)
			{
				prgCounter = (ReadMemory(indirectAddress - 0xFF) << 8) | ReadMemory(indirectAddress);
			}
			else
			{
				prgCounter = (ReadMemory(indirectAddress + 1) << 8) | ReadMemory(indirectAddress);
			}
		}
	}

	void RTI()
	{
		ReadMemory(prgCounter); // Dummy Read
		u8 flagValues = PopStack();
		flags = flagValues & 0xCF;

		u8 lowByte = PopStack();
		u8 highByte = PopStack();
		prgCounter = (highByte << 8) | lowByte;
	}

	void JSR()
	{
		ReadMemory(prgCounter); // Dummy Read
		u16 addressToPush = prgCounter - 1;
		u8 highByte = (addressToPush >> 8) & 0xFF;
		u8 lowByte = addressToPush & 0xFF;
		PushStack(highByte);
		PushStack(lowByte);
		prgCounter = operand;
	}

	void RTS()
	{
		u8 lowByte = PopStack();
		u8 highByte = PopStack();

		ReadMemory(prgCounter); // Dummy Read
		ReadMemory(prgCounter); // Dummy Read

		u16 address = (highByte << 8) | lowByte;
		prgCounter = address + 1;
	}

	void BranchByOffset()
	{
		// Signed value, so can be negative offset!!
		s8 offsetBy = (s8)operand;

		if(triggerIrq && !prevTriggerIrq)  { triggerIrq = false; }
		ReadMemory(prgCounter); // Dummy Read

		u16 newPrgCounter = prgCounter + offsetBy;
		if(IsCrossPageBoundary(newPrgCounter, prgCounter))
		{
			ReadMemory(prgCounter); // Dummy Read
		}
		prgCounter = newPrgCounter;
	}

	void BEQ() { if(IsFlagBitSet(ZERO_BIT)) { BranchByOffset(); } }
	void BNE() { if(!IsFlagBitSet(ZERO_BIT)) { BranchByOffset(); } }
	void BMI() { if(IsFlagBitSet(NEGATIVE_BIT)) { BranchByOffset(); } }
	void BPL() { if(!IsFlagBitSet(NEGATIVE_BIT)) { BranchByOffset(); } }
	void BVS() { if(IsFlagBitSet(OVERFLOW_BIT)) { BranchByOffset(); } }
	void BVC() { if(!IsFlagBitSet(OVERFLOW_BIT)) { BranchByOffset(); } }
	void BCS() { if(IsFlagBitSet(CARRY_BIT)) { BranchByOffset(); } }
	void BCC() { if(!IsFlagBitSet(CARRY_BIT)) { BranchByOffset(); } }


	// Unofficial ops

	void KIL() { Assert(0); } // No programs should call this
	void XAA() {}; // TODO: Implement? 
	void LAS() {}; // TODO: Implement? 
	void SHX() {}; // TODO: Implement? 
	void SHY() {}; // TODO: Implement? 
	void TAS() {}; // TODO: Implement? 
	void XXA() {}; // TODO: Implement? 

	void RRA()
	{
		u8 opValue = GetOpValue();
		WriteMemory(operand, opValue);
		// ROR
		u8 rorValue = 0;

		Assert(0); // TODO

		WriteMemory(operand, rorValue);
	}
	void SLO()
	{
	/*
	value = asl(value, cpu);
	value = ora(value, cpu);
	return(value);
	*/
	}

	void SRE()
	{
	/*
	value = lsr(value, cpu);
	value = eor(value, cpu);
	return(value);
	*/
	}

	void RLA()
	{
	/*
	value = rol(value, cpu);
	value = AND(value, cpu);
	return(value);
	*/
	}

	void LAX()
	{
		u8 opValue = GetOpValue();
		X = opValue;
		A = opValue;
		SetZero(A); // Can skip the Zero and negative flags for the X register. Overwritten immediately
		SetNegative(A);
	}

	void SAX() { WriteMemory(operand, A & X); }
	void SKB() {};
	void SKW() {};
	void AHX() {};
	void ALR() 
	{
		u8 opValue = GetOpValue();
		A = A & opValue;
		SetZero(A);
		SetNegative(A);
		if(A & 0x01) { SetCarry(); } else { ClearCarry(); }
		A = A >> 1;
		SetZero(A);
		SetNegative(A);
	};
	void ANC() 
	{
		u8 opValue = GetOpValue();
		A = A & opValue;
		SetZero(A);
		SetNegative(A);
		if(IsFlagBitSet(NEGATIVE_BIT)) { SetCarry(); } else { ClearCarry(); }
	};
	void ARR() 
	{
		u8 opValue = GetOpValue();
		u8 newA = (A & opValue) >> 1;
		if(IsFlagBitSet(CARRY_BIT)) { newA = newA | 0x80; }
		A = newA;
		
		SetZero(A);
		SetNegative(A);

		if(A & 0x40) { SetCarry(); } else { ClearCarry(); }

		bool isOverflow = (IsFlagBitSet(CARRY_BIT) ? 0x01 : 0) ^ ((A >> 5) & 0x01);
		if(isOverflow) { SetOverflow(); } else { ClearOverflow(); }
	};
	void AXS()
	{
		u8 opValue = GetOpValue();
		u8 andVal = A & X;
		u8 cmpVal = andVal - opValue;

		if(andVal >= opValue) { SetCarry(); } else { ClearCarry(); }
		X = cmpVal;
		SetZero(X);
		SetNegative(X);
	};

	void ISC()
	{
		u8 opValue = GetOpValue();
		WriteMemory(operand, opValue);
		opValue += 1;
		AddOperation(opValue ^ 0xFF);
		WriteMemory(operand, opValue);
	};

	void DCP()
	{
		u8 opValue = GetOpValue();
		WriteMemory(operand, opValue);
		opValue -= 1;
		//CMP
		u16 cmpValue = A - opValue;
		SetZero(cmpValue);
		SetNegative(cmpValue);
		if(A >= opValue) { SetCarry(); } else { ClearCarry(); }
		//
		WriteMemory(operand, opValue);
	}

	typedef void(Cpu:: *Operation)();
	Operation operations[INSTRUCTION_COUNT] =
	{
		/*          0          1          2          3          4          5          6          7          8          9          A          B          C          D          E          F   */
		/*0*/ &Cpu::BRK, &Cpu::ORA, &Cpu::KIL, &Cpu::SLO, &Cpu::SKB, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO, &Cpu::PHP, &Cpu::ORA, &Cpu::ASL, &Cpu::ANC, &Cpu::SKW, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO,
		/*1*/ &Cpu::BPL, &Cpu::ORA, &Cpu::KIL, &Cpu::SLO, &Cpu::SKB, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO, &Cpu::CLC, &Cpu::ORA, &Cpu::NOP, &Cpu::SLO, &Cpu::SKW, &Cpu::ORA, &Cpu::ASL, &Cpu::SLO,
		/*2*/ &Cpu::JSR, &Cpu::AND, &Cpu::KIL, &Cpu::RLA, &Cpu::BIT, &Cpu::AND, &Cpu::ROL, &Cpu::RLA, &Cpu::PLP, &Cpu::AND, &Cpu::ROL, &Cpu::ANC, &Cpu::BIT, &Cpu::AND, &Cpu::ROL, &Cpu::RLA,
		/*3*/ &Cpu::BMI, &Cpu::AND, &Cpu::KIL, &Cpu::RLA, &Cpu::SKB, &Cpu::AND, &Cpu::ROL, &Cpu::RLA, &Cpu::SEC, &Cpu::AND, &Cpu::NOP, &Cpu::RLA, &Cpu::SKW, &Cpu::AND, &Cpu::ROL, &Cpu::RLA,
		/*4*/ &Cpu::RTI, &Cpu::EOR, &Cpu::KIL, &Cpu::SRE, &Cpu::SKB, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE, &Cpu::PHA, &Cpu::EOR, &Cpu::LSR, &Cpu::ALR, &Cpu::JMP, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE,
		/*5*/ &Cpu::BVC, &Cpu::EOR, &Cpu::KIL, &Cpu::SRE, &Cpu::SKB, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE, &Cpu::CLI, &Cpu::EOR, &Cpu::NOP, &Cpu::SRE, &Cpu::SKW, &Cpu::EOR, &Cpu::LSR, &Cpu::SRE,
		/*6*/ &Cpu::RTS, &Cpu::ADC, &Cpu::KIL, &Cpu::RRA, &Cpu::SKB, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA, &Cpu::PLA, &Cpu::ADC, &Cpu::ROR, &Cpu::ARR, &Cpu::JMP, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA,
		/*7*/ &Cpu::BVS, &Cpu::ADC, &Cpu::KIL, &Cpu::RRA, &Cpu::SKB, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA, &Cpu::SEI, &Cpu::ADC, &Cpu::NOP, &Cpu::RRA, &Cpu::SKW, &Cpu::ADC, &Cpu::ROR, &Cpu::RRA,
		/*8*/ &Cpu::SKB, &Cpu::STA, &Cpu::SKB, &Cpu::SAX, &Cpu::STY, &Cpu::STA, &Cpu::STX, &Cpu::SAX, &Cpu::DEY, &Cpu::SKB, &Cpu::TXA, &Cpu::XAA, &Cpu::STY, &Cpu::STA, &Cpu::STX, &Cpu::SAX,
		/*9*/ &Cpu::BCC, &Cpu::STA, &Cpu::KIL, &Cpu::AHX, &Cpu::STY, &Cpu::STA, &Cpu::STX, &Cpu::SAX, &Cpu::TYA, &Cpu::STA, &Cpu::TXS, &Cpu::TAS, &Cpu::SHY, &Cpu::STA, &Cpu::SHX, &Cpu::AHX,
		/*A*/ &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX, &Cpu::TAY, &Cpu::LDA, &Cpu::TAX, &Cpu::LAX, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX,
		/*B*/ &Cpu::BCS, &Cpu::LDA, &Cpu::KIL, &Cpu::LAX, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX, &Cpu::CLV, &Cpu::LDA, &Cpu::TSX, &Cpu::LAS, &Cpu::LDY, &Cpu::LDA, &Cpu::LDX, &Cpu::LAX,
		/*C*/ &Cpu::CPY, &Cpu::CMP, &Cpu::SKB, &Cpu::DCP, &Cpu::CPY, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP, &Cpu::INY, &Cpu::CMP, &Cpu::DEX, &Cpu::AXS, &Cpu::CPY, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP,
		/*D*/ &Cpu::BNE, &Cpu::CMP, &Cpu::KIL, &Cpu::DCP, &Cpu::SKB, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP, &Cpu::CLD, &Cpu::CMP, &Cpu::NOP, &Cpu::DCP, &Cpu::SKW, &Cpu::CMP, &Cpu::DEC, &Cpu::DCP,
		/*E*/ &Cpu::CPX, &Cpu::SBC, &Cpu::SKB, &Cpu::ISC, &Cpu::CPX, &Cpu::SBC, &Cpu::INC, &Cpu::ISC, &Cpu::INX, &Cpu::SBC, &Cpu::NOP, &Cpu::SBC, &Cpu::CPX, &Cpu::SBC, &Cpu::INC, &Cpu::ISC,
		/*F*/ &Cpu::BEQ, &Cpu::SBC, &Cpu::KIL, &Cpu::ISC, &Cpu::SKB, &Cpu::SBC, &Cpu::INC, &Cpu::ISC, &Cpu::SED, &Cpu::SBC, &Cpu::NOP, &Cpu::ISC, &Cpu::SKW, &Cpu::SBC, &Cpu::INC, &Cpu::ISC
	};
};