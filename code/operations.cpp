#include "nes.h"

#include "cpu.h"

// Implied Operations

u8 clc(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	ClearCarry(&cpu->flags);
	return(0);
}
u8 cld(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	ClearDecimal(&cpu->flags);
	return(0);
}
u8 cli(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	ClearInterrupt(&cpu->flags);
	return(0);
}
u8 clv(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	ClearOverflow(&cpu->flags);
	return(0);
}

u8 dex(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	--cpu->X;
	SetZero(cpu->X, &cpu->flags);
	SetNegative(cpu->X, &cpu->flags);
	return(0);
}
u8 dey(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	--cpu->Y;
	SetZero(cpu->Y, &cpu->flags);
	SetNegative(cpu->Y, &cpu->flags);
	return(0);
}

u8 inx(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	++cpu->X;
	SetZero(cpu->X, &cpu->flags);
	SetNegative(cpu->X, &cpu->flags);
	return(0);
}
u8 iny(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	++cpu->Y;
	SetZero(cpu->Y, &cpu->flags);
	SetNegative(cpu->Y, &cpu->flags);
	return(0);
}

u8 sec(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	SetCarry(&cpu->flags);
	return(0);
}
u8 sed(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	SetDecimal(&cpu->flags);
	return(0);
}
u8 sei(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	SetInterrupt(&cpu->flags);
	return(0);
}

u8 tax(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	cpu->X = cpu->A;
	SetZero(cpu->X, &cpu->flags);
	SetNegative(cpu->X, &cpu->flags);
	return(0);
}
u8 txa(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	cpu->A = cpu->X;
	SetZero(cpu->A, &cpu->flags);
	SetNegative(cpu->A, &cpu->flags);
	return(0);
}

u8 tay(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	cpu->Y = cpu->A;
	SetZero(cpu->Y, &cpu->flags);
	SetNegative(cpu->Y, &cpu->flags);
	return(0);
}
u8 tya(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	cpu->A = cpu->Y;
	SetZero(cpu->A, &cpu->flags);
	SetNegative(cpu->A, &cpu->flags);
	return(0);
}

u8 tsx(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	cpu->X = cpu->stackPtr;
	SetZero(cpu->X, &cpu->flags);
	SetNegative(cpu->X, &cpu->flags);
	return(0);
}
u8 txs(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	cpu->stackPtr = cpu->X;
	return(0);
}

u8 nop(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	// Does nothing    
	return(0);
}

// TODO: Condense irq and nmi in future
u8 irq(u8 value, Cpu *cpu)
{
	WriteStack((cpu->prgCounter >> 8), cpu);
	WriteStack((cpu->prgCounter & 0xFF), cpu);
	// At this point, which interrupt is determined. Can be hijacked
	// Current implementation is just separating the nmi irq brk functions, may change // Cycle 5
	ClearBreak(&cpu->flags);
	WriteStack(cpu->flags, cpu);
	cpu->prgCounter = (cpu->prgCounter & 0xFF00) | Read8(cpu->memory + IRQ_BRK_VEC, 6); // Cycle 6      
	cpu->prgCounter = (Read8(cpu->memory + (IRQ_BRK_VEC + 1), 7) << 8) | (cpu->prgCounter & 0xFF); // Cycle 7
	ClearBreak(&cpu->flags);
	SetInterrupt(&cpu->flags);
	return(0);
}


u8 nmi(u8 value, Cpu *cpu)
{
	WriteStack((cpu->prgCounter >> 8), cpu);
	WriteStack((cpu->prgCounter & 0xFF), cpu);
	// At this point, which interrupt is determined. Can be hijacked
	// Current implementation is just separating the nmi irq brk functions, may change // Cycle 5
	ClearBreak(&cpu->flags);
	WriteStack(cpu->flags, cpu);
	cpu->prgCounter = (cpu->prgCounter & 0xFF00) | Read8(cpu->memory + NMI_VEC, 6);     // Cycle 6    
	cpu->prgCounter = (Read8(cpu->memory + (NMI_VEC + 1), 7) << 8) | (cpu->prgCounter & 0xFF); // Cycle 7
	ClearBreak(&cpu->flags);
	SetInterrupt(&cpu->flags);
	return(0);
}

u8 brk(u8 value, Cpu *cpu)
{
	ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2
	u8 HighByte = (u8)(cpu->prgCounter >> 8);
	WriteStack(HighByte, cpu);
	u8 LowByte = (u8)cpu->prgCounter;
	WriteStack(LowByte, cpu);
	PollInterrupts(cpu, 4); // Cycle 4
	// TODO: NMI Hijack  // Cycle 4
	SetBlank(&cpu->flags);
	SetBreak(&cpu->flags);
	WriteStack(cpu->flags, cpu);
	SetInterrupt(&cpu->flags);
	cpu->prgCounter = (cpu->prgCounter & 0xFF00) | Read8(cpu->memory + IRQ_BRK_VEC, 6); // Cycle 6
	cpu->prgCounter = (Read8(cpu->memory + (IRQ_BRK_VEC + 1), 7) << 8) | (cpu->prgCounter & 0x00FF); // Cycle 7
	return(0);
}

u8 rti(u8 value, Cpu *cpu)
{
	ReadCpu8(cpu->prgCounter, cpu, 2); // Cycle 2
	cpu->flags = readStack(cpu); // Cycle 4
	cpu->prgCounter = (cpu->prgCounter & 0xFF00) | readStack(cpu);
	PollInterrupts(cpu, 5); // Cycle 5
	cpu->prgCounter = (readStack(cpu) << 8) | (cpu->prgCounter & 0x00FF);
	return(0);
}

u8 rts(u8 value, Cpu *cpu)
{
	ReadCpu8(cpu->prgCounter, cpu, 2); // Cycle 2
	cpu->prgCounter = cpu->prgCounter & 0xFF00 | readStack(cpu);
	cpu->prgCounter = (readStack(cpu) << 8) | (cpu->prgCounter & 0x00FF);
	PollInterrupts(cpu, 5); // Cycle 5
	++cpu->prgCounter;
	return(0);
}

u8 pha(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 2); // Cycle 2
	WriteStack(cpu->A, cpu);
	return(0);
}

u8 php(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 2); // Cycle 2
	SetBreak(&cpu->flags);
	SetBlank(&cpu->flags);
	WriteStack(cpu->flags, cpu);
	return(0);
}

u8 pla(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 3); // Cycle 3
	cpu->A = readStack(cpu);
	SetZero(cpu->A, &cpu->flags);
	SetNegative(cpu->A, &cpu->flags);
	return(0);
}

u8 plp(u8 value, Cpu *cpu)
{
	PollInterrupts(cpu, 3); // Cycle 3
	cpu->flags = readStack(cpu);
	return(0);
}

u8 jsr(u8 value, Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2
	// TODO: The order of this seems off? Getting low byte first
	// then writing to stack followed by the high byte read??

	// TODO: No Poll interrupts?
	WriteStack((cpu->prgCounter >> 8), cpu); // TODO: Combine decrement and WriteStack
	WriteStack(cpu->prgCounter & 0xFF, cpu);
	PollInterrupts(cpu, 5);
	cpu->prgCounter = (ReadCpu8(cpu->prgCounter, cpu, 6) << 8) | cpu->opLowByte; // Cycle 6

	return(0);
}

// Read
u8 lda(u8 value, Cpu *cpu)
{
	cpu->A = value;
	SetZero(cpu->A, &cpu->flags);
	SetNegative(cpu->A, &cpu->flags);
	return(0);
}
u8 ldx(u8 value, Cpu *cpu)
{
	cpu->X = value;
	SetZero(cpu->X, &cpu->flags);
	SetNegative(cpu->X, &cpu->flags);
	return(0);
}
u8 ldy(u8 value, Cpu *cpu)
{
	cpu->Y = value;
	SetZero(cpu->Y, &cpu->flags);
	SetNegative(cpu->Y, &cpu->flags);
	return(0);
}
u8 eor(u8 value, Cpu *cpu)
{
	cpu->A = cpu->A ^ value;
	SetZero(cpu->A, &cpu->flags);
	SetNegative(cpu->A, &cpu->flags);
	return(value);
}
u8 AND(u8 value, Cpu *cpu)
{
	cpu->A = cpu->A & value;
	SetZero(cpu->A, &cpu->flags);
	SetNegative(cpu->A, &cpu->flags);
	return(value);
}

u8 ora(u8 value, Cpu *cpu)
{
	cpu->A = cpu->A | value;
	SetZero(cpu->A, &cpu->flags);
	SetNegative(cpu->A, &cpu->flags);
	return(value);
}

u8 adc(u8 value, Cpu *cpu)
{
	u8 A = cpu->A;
	u8 B = value;
	u8 C = IsBitSet(CARRY_BIT, cpu->flags);

	u16 Sum = (u16)A + (u16)B + (u16)C;

	// Overflow check, taken from the web. One day find out how this works
	if(((A ^ Sum) & (B ^ Sum) & 0x80) == 0x80)
		SetOverflow(&cpu->flags);
	else
		ClearOverflow(&cpu->flags);

	if(Sum & 0x100)
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);

	// TODO: Check this cast, sum has a negative flag on the 16th bit??
	SetZero((u8)Sum, &cpu->flags);
	SetNegative((u8)Sum, &cpu->flags);

	cpu->A = (u8)Sum;
	return(0);
}

u8 sbc(u8 value, Cpu *cpu)
{
	u8 A = cpu->A;
	u8 B = ~value; // NOTE: Using the inverse
	u8 C = IsBitSet(CARRY_BIT, cpu->flags);

	u16 Sum = (u16)A + (u16)B + (u16)C;

	// Overflow check, taken from the web. One day find out how this works
	if(((A ^ Sum) & (B ^ Sum) & 0x80) == 0x80)
		SetOverflow(&cpu->flags);
	else
		ClearOverflow(&cpu->flags);

	if(Sum & 0x100)
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);

	SetZero((u8)Sum, &cpu->flags);
	SetNegative((u8)Sum, &cpu->flags);

	cpu->A = (u8)Sum;

	return(0);
}

u8 cmp(u8 value, Cpu *cpu)
{
	if(cpu->A >= value)
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);

	u8 Cmpvalue = cpu->A - value;
	SetZero(Cmpvalue, &cpu->flags);
	SetNegative(Cmpvalue, &cpu->flags);
	return(0);
}

u8 bit(u8 value, Cpu *cpu)
{
	if(value & (1 << 6))
		SetOverflow(&cpu->flags);
	else
		ClearOverflow(&cpu->flags);
	SetNegative(value, &cpu->flags);
	SetZero(cpu->A & value, &cpu->flags);
	return(0);
}

u8 lax(u8 value, Cpu *cpu)
{
	lda(value, cpu);
	ldx(value, cpu);

	return(0);
}



// Read modify write
u8 asl(u8 value, Cpu *cpu)
{
	if(value & (1 << 7))
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);
	value = value << 1;

	SetZero(value, &cpu->flags);
	SetNegative(value, &cpu->flags);

	return(value);
}

u8 lsr(u8 value, Cpu *cpu)
{
	if(value & 1)
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);
	value = value >> 1;

	SetZero(value, &cpu->flags);
	SetNegative(value, &cpu->flags);

	return(value);
}

u8 rol(u8 value, Cpu *cpu)
{
	u8 CarrySet = IsBitSet(CARRY_BIT, cpu->flags);

	if(value & (1 << 7))
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);
	value = value << 1;

	if(CarrySet)
		value = value | 1;

	SetZero(value, &cpu->flags);
	SetNegative(value, &cpu->flags);

	return(value);
}

u8 ror(u8 value, Cpu *cpu)
{
	u8 CarrySet = IsBitSet(CARRY_BIT, cpu->flags);

	if(value & 1)
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);
	value = value >> 1;

	if(CarrySet)
		value = value | (1 << 7);

	SetZero(value, &cpu->flags);
	SetNegative(value, &cpu->flags);

	return(value);
}

u8 inc(u8 value, Cpu *cpu)
{
	++value;
	SetZero(value, &cpu->flags);
	SetNegative(value, &cpu->flags);
	return(value);
}

u8 dec(u8 value, Cpu *cpu)
{
	--value;
	SetZero(value, &cpu->flags);
	SetNegative(value, &cpu->flags);
	return(value);
}

u8 slo(u8 value, Cpu *cpu)
{
	value = asl(value, cpu);
	value = ora(value, cpu);
	return(value);
}
u8 sre(u8 value, Cpu *cpu)
{
	value = lsr(value, cpu);
	value = eor(value, cpu);
	return(value);
}

u8 rla(u8 value, Cpu *cpu)
{
	value = rol(value, cpu);
	value = AND(value, cpu);
	return(value);
}
u8 rra(u8 value, Cpu *cpu)
{
	value = ror(value, cpu);
	adc(value, cpu);
	return(value);
}

u8 isc(u8 value, Cpu *cpu)
{
	value = inc(value, cpu);
	sbc(value, cpu);
	return(value);
}

u8 dcp(u8 value, Cpu *cpu)
{
	value = dec(value, cpu);
	cmp(value, cpu);
	return(value);
}

// Write instructions

u8 sta(u8 value, Cpu *cpu)
{
	return(cpu->A);
}
u8 stx(u8 value, Cpu *cpu)
{
	return(cpu->X);
}
u8 sty(u8 value, Cpu *cpu)
{
	return(cpu->Y);
}
u8 sax(u8 value, Cpu *cpu)
{
	return(cpu->A & cpu->X);
}


/////////////////////////
// Relative Operations //
/////////////////////////

u8 bcs(u8 value, Cpu *cpu)
{
	return(IsBitSet(CARRY_BIT, cpu->flags));
}
u8 bcc(u8 value, Cpu *cpu)
{
	return(!IsBitSet(CARRY_BIT, cpu->flags));
}

u8 beq(u8 value, Cpu *cpu)
{
	return(IsBitSet(ZERO_BIT, cpu->flags));
}
u8 bne(u8 value, Cpu *cpu)
{
	return(!IsBitSet(ZERO_BIT, cpu->flags));
}

u8 bmi(u8 value, Cpu *cpu)
{
	return(IsBitSet(NEGATIVE_BIT, cpu->flags));
}
u8 bpl(u8 value, Cpu *cpu)
{
	return(!IsBitSet(NEGATIVE_BIT, cpu->flags));
}

u8 bvs(u8 value, Cpu *cpu)
{
	return(IsBitSet(OVERFLOW_BIT, cpu->flags));
}
u8 bvc(u8 value, Cpu *cpu)
{
	return(!IsBitSet(OVERFLOW_BIT, cpu->flags));
}
/////////////////////


u8 jmp(u8 value, Cpu *cpu)
{
	cpu->prgCounter = (cpu->opHighByte << 8) | cpu->opLowByte;
	return(0);
}

// Compares

u8 cpx(u8 value, Cpu *cpu)
{
	if(cpu->X >= value)
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);

	u8 Cmpvalue = cpu->X - value;
	SetZero(Cmpvalue, &cpu->flags);
	SetNegative(Cmpvalue, &cpu->flags);
	return(0);
}
u8 cpy(u8 value, Cpu *cpu)
{
	if(cpu->Y >= value)
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);

	u8 Cmpvalue = cpu->Y - value;
	SetZero(Cmpvalue, &cpu->flags);
	SetNegative(Cmpvalue, &cpu->flags);
	return(0);
}


u8 skb(u8 value, Cpu *cpu)
{
	// Size 2
	return(0);
}

u8 skw(u8 value, Cpu *cpu)
{
	// Size 3
	return(0);
}

u8 ahx(u8 value, Cpu *cpu)
{
	//Assert(0);
	return(0);
}
u8 alr(u8 value, Cpu *cpu)
{
	value = AND(value, cpu);
	value = lsr(value, cpu);
	return(value);
}
u8 anc(u8 value, Cpu *cpu)
{
	AND(value, cpu);

	if(IsBitSet(NEGATIVE_BIT, cpu->flags))
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);

	return(0);
}
u8 arr(u8 value, Cpu *cpu)
{
	AND(value, cpu);
	ror(value, cpu);

	u8 bit5 = cpu->A & (1 << 5);
	u8 bit6 = cpu->A & (1 << 6);

	if(bit6)
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);

	if((bit5 && !bit6) || (!bit5 && bit6))
		SetOverflow(&cpu->flags);
	else
		ClearOverflow(&cpu->flags);

	SetZero(cpu->A, &cpu->flags);
	SetNegative(cpu->A, &cpu->flags);

	return(0);
}
u8 axs(u8 value, Cpu *cpu)
{
	u8 ANDvalue = (cpu->A & cpu->X);
	cpu->X = ANDvalue - value;

	if(ANDvalue >= value)
		SetCarry(&cpu->flags);
	else
		ClearCarry(&cpu->flags);

	SetZero(cpu->X, &cpu->flags);
	SetNegative(cpu->X, &cpu->flags);

	return(0);
}
u8 kil(u8 value, Cpu *cpu)
{
	Assert(0);
	return(0);
}
u8 las(u8 value, Cpu *cpu)
{
	//Assert(0);
	return(0);
}
u8 shx(u8 value, Cpu *cpu)
{
	//Assert(0);
	return(0);
}

u8 shy(u8 value, Cpu *cpu)
{
	//Assert(0);
//    if((cpu->X + value) <= 0xFF)
	//WriteCpu8(Byte, Address, cpu); // TODO: This shouldn't happen, need refactoring
	return(0);
}
u8 tas(u8 value, Cpu *cpu)
{
	//Assert(0);
	return(0);
}

u8 xaa(u8 value, Cpu *cpu)
{
	//Assert(0);
	return(0);
}



static u8 OpAddressType[INSTRUCTION_COUNT] =
{
	/*         0        1      2        3       4       5        6        7     8       9     A        B       C     D     E     F  */
	/*0*/   IMPL,  INDX_R,  IMPL, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED,  ACM,    IMED,  ABS_R,  ABS_R,  ABS_RW,  ABS_RW,
	/*1*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
	/*2*/   IMPL,  INDX_R,  IMPL, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED,  ACM,    IMED,  ABS_R,  ABS_R,  ABS_RW,  ABS_RW,
	/*3*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
	/*4*/   IMPL,  INDX_R,  IMPL, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED,  ACM,    IMED,   ABSJ,  ABS_R,  ABS_RW,  ABS_RW,
	/*5*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
	/*6*/   IMPL,  INDX_R,  IMPL, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED,  ACM,    IMED,   INDI,  ABS_R,  ABS_RW,  ABS_RW,
	/*7*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
	/*8*/   IMED,  INDX_W,  IMED,  INDX_W, ZERO_W, ZERO_W,  ZERO_W,  ZERO_W, IMPL,   IMED, IMPL,    IMED,  ABS_W,  ABS_W,   ABS_W,   ABS_W,
	/*9*/    REL,  INDY_W,  IMPL,  INDY_W, ZERX_W, ZERX_W,  ZERY_W,  ZERY_W, IMPL, ABSY_W, IMPL,  ABSY_W, ABSX_W, ABSX_W,  ABSY_W,  ABSY_W,
	/*A*/   IMED,  INDX_R,  IMED,  INDX_R, ZERO_R, ZERO_R,  ZERO_R,  ZERO_R, IMPL,   IMED, IMPL,    IMED,  ABS_R,  ABS_R,   ABS_R,   ABS_R,
	/*B*/    REL,  INDY_R,  IMPL,  INDY_R, ZERX_R, ZERX_R,  ZERY_R,  ZERY_R, IMPL, ABSY_R, IMPL,  ABSY_R, ABSX_R, ABSX_R,  ABSY_R,  ABSY_R,
	/*C*/   IMED,  INDX_R,  IMED, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED, IMPL,    IMED,  ABS_R,  ABS_R,  ABS_RW,  ABS_RW,
	/*D*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
	/*E*/   IMED,  INDX_R,  IMED, INDX_RW, ZERO_R, ZERO_R, ZERO_RW, ZERO_RW, IMPL,   IMED, IMPL,    IMED,  ABS_R,  ABS_R,  ABS_RW,  ABS_RW,
	/*F*/    REL,  INDY_R,  IMPL, INDY_RW, ZERX_R, ZERX_R, ZERX_RW, ZERX_RW, IMPL, ABSY_R, IMPL, ABSY_RW, ABSX_R, ABSX_R, ABSX_RW, ABSX_RW,
};

static char *OpName[INSTRUCTION_COUNT] =
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

static u8 OpClocks[INSTRUCTION_COUNT] =
{
	/*     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F     */
	/*0*/  7, 6, 7, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
	/*1*/  2, 5, 7, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	/*2*/  6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
	/*3*/  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	/*4*/  6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
	/*5*/  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	/*6*/  6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
	/*7*/  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	/*8*/  2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
	/*9*/  2, 6, 0, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
	/*A*/  2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
	/*B*/  2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
	/*C*/  2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
	/*D*/  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	/*E*/  2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
	/*F*/  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
};

u8(*Operations[INSTRUCTION_COUNT])(u8 inByte, Cpu *cpu) =
{
	/*         0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F        */
	/*0*/    brk,ora,nmi,slo,skb,ora,asl,slo,php,ora,asl,anc,skw,ora,asl,slo,
	/*1*/    bpl,ora,irq,slo,skb,ora,asl,slo,clc,ora,nop,slo,skw,ora,asl,slo,
	/*2*/    jsr,AND,kil,rla,bit,AND,rol,rla,plp,AND,rol,anc,bit,AND,rol,rla,
	/*3*/    bmi,AND,kil,rla,skb,AND,rol,rla,sec,AND,nop,rla,skw,AND,rol,rla,
	/*4*/    rti,eor,kil,sre,skb,eor,lsr,sre,pha,eor,lsr,alr,jmp,eor,lsr,sre,
	/*5*/    bvc,eor,kil,sre,skb,eor,lsr,sre,cli,eor,nop,sre,skw,eor,lsr,sre,
	/*6*/    rts,adc,kil,rra,skb,adc,ror,rra,pla,adc,ror,arr,jmp,adc,ror,rra,
	/*7*/    bvs,adc,kil,rra,skb,adc,ror,rra,sei,adc,nop,rra,skw,adc,ror,rra,
	/*8*/    skb,sta,skb,sax,sty,sta,stx,sax,dey,skb,txa,xaa,sty,sta,stx,sax,
	/*9*/    bcc,sta,kil,ahx,sty,sta,stx,sax,tya,sta,txs,tas,shy,sta,shx,ahx,
	/*A*/    ldy,lda,ldx,lax,ldy,lda,ldx,lax,tay,lda,tax,lax,ldy,lda,ldx,lax,
	/*B*/    bcs,lda,kil,lax,ldy,lda,ldx,lax,clv,lda,tsx,las,ldy,lda,ldx,lax,
	/*C*/    cpy,cmp,skb,dcp,cpy,cmp,dec,dcp,iny,cmp,dex,axs,cpy,cmp,dec,dcp,
	/*D*/    bne,cmp,kil,dcp,skb,cmp,dec,dcp,cld,cmp,nop,dcp,skw,cmp,dec,dcp,
	/*E*/    cpx,sbc,skb,isc,cpx,sbc,inc,isc,inx,sbc,nop,sbc,cpx,sbc,inc,isc,
	/*F*/    beq,sbc,kil,isc,skb,sbc,inc,isc,sed,sbc,nop,isc,skw,sbc,inc,isc
};


void implied(Cpu *cpu)
{
	Operations[cpu->opCode](0, cpu);
}

//////////////////////////////////////
//// ACCUMULATOR AND IMPLIED MODE ////
//////////////////////////////////////

// NOTE: All accumulator ops are 2 cycles
void accumulator(Cpu *cpu)
{
	PollInterrupts(cpu, 1); // CYcle 1
	cpu->A = Operations[cpu->opCode](cpu->A, cpu);
}

////////////////////////
//// IMMEDIATE MODE ////
////////////////////////

// NOTE: 2 Cycles
void immediate(Cpu *cpu)
{
	PollInterrupts(cpu, 1); // Cycle 1
	u8 value = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2
	Operations[cpu->opCode](value, cpu);
}


///////////////////////
//// RELATIVE MODE ////
///////////////////////

// NOTE: Cycles change depending if branched, and if pages crossed.
void relative(Cpu *cpu)
{
	PollInterrupts(cpu, 2); // Cycle 2
	cpu->opValue = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	// Cycle 3    
	// The Correct Program Counter. Will be saved into OpHigh and Low Bytes
	u16 TempPC = cpu->prgCounter + (s8)cpu->opValue;
	cpu->opHighByte = (TempPC & 0xFF00) >> 8;
	cpu->opLowByte = TempPC & 0x00FF;

	// The potentially wrong high byte after branch
	u8 UnFixedHighByte = (cpu->prgCounter & 0xFF00) >> 8;

	u8 TakeBranch = Operations[cpu->opCode](0, cpu);

	if(TakeBranch)
	{
		cpu->opClockTotal += 1;

		// Update prg counter, could have unfixed page
		cpu->prgCounter = (cpu->prgCounter & 0xFF00) | cpu->opLowByte;

		// If opHighByte isn't the same as the unfixed version, then we crossed page
		if(cpu->opHighByte != UnFixedHighByte)
		{
			// Page crossed so fix
			cpu->opClockTotal += 1;
			PollInterrupts(cpu, 4); // Cycle 4
			cpu->prgCounter = (cpu->opHighByte << 8) | (cpu->prgCounter & 0xFF);
		}
	}
}

////////////////////////
//// ZERO PAGE MODE ////
////////////////////////

// NOTE: 3 Cycles
void zeroRead(Cpu *cpu)
{
	PollInterrupts(cpu, 2); // Cycle 2
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	u8 Value = ReadCpu8(cpu->opLowByte, cpu, 3); // Cycle 3
	Operations[cpu->opCode](Value, cpu);
}

// NOTE: 5 Cycles
void zeroReadWrite(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2 

	cpu->opValue = ReadCpu8(cpu->opLowByte, cpu, 3); // Cycle 3
	cpu->opValue = Operations[cpu->opCode](cpu->opValue, cpu);
	PollInterrupts(cpu, 4); // Cycle 4
	WriteCpu8(cpu->opValue, cpu->opLowByte, cpu, 5); // Cycle 5
}

// NOTE: 3 Cycles
void zeroWrite(Cpu *cpu)
{
	PollInterrupts(cpu, 2); // Cycle 2
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2 

	u8 Value = Operations[cpu->opCode](0, cpu);
	WriteCpu8(Value, cpu->opLowByte, cpu, 3); // Cycle 3
}

//////////////////////////
//// ZERO PAGE X MODE ////
//////////////////////////

// 4 cycles
void zeroXIndexRead(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opLowByte += cpu->X;

	PollInterrupts(cpu, 3); // Cycle 3
	u8 Value = ReadCpu8(cpu->opLowByte, cpu, 4); // Cycle 4
	Operations[cpu->opCode](Value, cpu);
}

// 6 cycles
void zeroXIndexReadWrite(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opLowByte += cpu->X;
	cpu->opValue = ReadCpu8(cpu->opLowByte, cpu, 4); // Cycle 4 

	PollInterrupts(cpu, 5); // Cycle 5
	cpu->opValue = Operations[cpu->opCode](cpu->opValue, cpu);
	WriteCpu8(cpu->opValue, cpu->opLowByte, cpu, 6); // Cycle 6
}

// 4 cycles
void zeroXIndexWrite(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opLowByte += cpu->X;
	PollInterrupts(cpu, 3); // Cycle 3

	u8 Value = Operations[cpu->opCode](0, cpu);
	WriteCpu8(Value, cpu->opLowByte, cpu, 4); // Cycle 4
}

//////////////////////////
//// ZERO PAGE Y MODE ////
//////////////////////////

// 4 cycles
void zeroYIndexRead(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opLowByte += cpu->Y;
	PollInterrupts(cpu, 3); // Cycle 3
	u8 Value = ReadCpu8(cpu->opLowByte, cpu, 4); // Cycle 4
	Operations[cpu->opCode](Value, cpu);
}

// 6 cycles
void zeroYIndexReadWrite(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opLowByte += cpu->Y;
	cpu->opValue = ReadCpu8(cpu->opLowByte, cpu, 4); // Cycle 4

	cpu->opValue = Operations[cpu->opCode](cpu->opValue, cpu);
	PollInterrupts(cpu, 5); // Cycle 5
	WriteCpu8(cpu->opValue, cpu->opLowByte, cpu, 6); // Cycle 6
}

// 4 cycles
void zeroYIndexWrite(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opLowByte += cpu->Y;
	PollInterrupts(cpu, 3); // Cycle 3

	u8 Value = Operations[cpu->opCode](0, cpu);
	WriteCpu8(Value, cpu->opLowByte, cpu, 4); // Cycle 4
}

///////////////////////
//// ABSOLUTE MODE ////
///////////////////////

// 3 cycles
void absJmp(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2
	cpu->opHighByte = ReadCpu8(cpu->prgCounter++, cpu, 3); // Cycle 3

	PollInterrupts(cpu, 2); // Cycle 2
	Operations[cpu->opCode](0, cpu);
}

// 4 cycles
void absRead(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2
	cpu->opHighByte = ReadCpu8(cpu->prgCounter++, cpu, 3); // Cycle 3

	PollInterrupts(cpu, 3); // Cycle 3
	u8 Value = ReadCpu8(((cpu->opHighByte << 8) | cpu->opLowByte), cpu, 4); // Cycle 4
	Operations[cpu->opCode](Value, cpu);
}

// 6 cycles
void absReadWrite(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2
	cpu->opHighByte = ReadCpu8(cpu->prgCounter++, cpu, 3); // Cycle 3

	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 4); // Cycle 4

	cpu->opValue = Operations[cpu->opCode](cpu->opValue, cpu);
	PollInterrupts(cpu, 5); // Cycle 5
	WriteCpu8(cpu->opValue, (cpu->opHighByte << 8) | cpu->opLowByte, cpu, 6); // CYcle 6
}

// 4 cycles
void absWrite(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2
	cpu->opHighByte = ReadCpu8(cpu->prgCounter++, cpu, 3); // Cycle 3

	PollInterrupts(cpu, 3); // Cycle 3

	u8 Value = Operations[cpu->opCode](0, cpu);
	WriteCpu8(Value, (cpu->opHighByte << 8) | cpu->opLowByte, cpu, 4); // Cycle 4
}

///////////////////////////////
//// ABSOLUTE X INDEX MODE ////
///////////////////////////////

// Cycles 4 or 5 page fix
void absXIndexRead(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2


	cpu->opHighByte = ReadCpu8(cpu->prgCounter++, cpu, 3); // Cycle 3
	cpu->opTemp = cpu->opLowByte;
	cpu->opLowByte += cpu->X;


	u8 Value = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 4); // Cycle 4

	if(cpu->opLowByte < cpu->opTemp) // If the page was crossed then fix.
	{
		cpu->opClockTotal += 1;
		++cpu->opHighByte;
		PollInterrupts(cpu, 4); // Cycle 4
		u8 Value = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 5); // Cycle 5
		Operations[cpu->opCode](Value, cpu);
	}
	else // else read was fine, execute instruction, end op
	{
		PollInterrupts(cpu, 3); // Cycle 3
		Operations[cpu->opCode](Value, cpu);
	}
}

// 7 cycles
void absXIndexReadWrite(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opHighByte = ReadCpu8(cpu->prgCounter++, cpu, 3); // Cycle 3
	cpu->opTemp = cpu->opLowByte;
	cpu->opLowByte += cpu->X;

	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 4); // Cycle 4

	if(cpu->opLowByte < cpu->opTemp) // If the page was crossed then fix.
	{
		++cpu->opHighByte;
	}

	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 5); // Cycle 5

	cpu->opValue = Operations[cpu->opCode](cpu->opValue, cpu);
	PollInterrupts(cpu, 6); // Cycle 6
	WriteCpu8(cpu->opValue, (cpu->opHighByte << 8) | cpu->opLowByte, cpu, 7); // Cycle 7
}

// 5 cycles
void absXIndexWrite(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opHighByte = ReadCpu8(cpu->prgCounter++, cpu, 3); // Cycle 3
	cpu->opTemp = cpu->opLowByte;
	cpu->opLowByte += cpu->X;

	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 4); // Cycle 4

	if(cpu->opLowByte < cpu->opTemp) // If the page was crossed then fix.
	{
		++cpu->opHighByte;
	}

	PollInterrupts(cpu, 4); // Cycle 4
	u8 Value = Operations[cpu->opCode](0, cpu);
	WriteCpu8(Value, (cpu->opHighByte << 8) | cpu->opLowByte, cpu, 5); // Cycle 5

}

///////////////////////////////
//// ABSOLUTE Y INDEX MODE ////
///////////////////////////////

// 4 or 5 cycles, fixed page dependant
void absYIndexRead(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2
	cpu->opHighByte = ReadCpu8(cpu->prgCounter++, cpu, 3); // Cycle 3

	cpu->opTemp = cpu->opLowByte;
	cpu->opLowByte += cpu->Y;
	u8 Value = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 4); // Cycle 4

	if(cpu->opLowByte < cpu->opTemp) // If the page was crossed then fix.
	{
		cpu->opClockTotal += 1;
		++cpu->opHighByte;
		PollInterrupts(cpu, 4); // Cycle 4
		// NOTE: Only reaches here if the page was crossed
		u8 Value = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 5); // Cycle 5
		Operations[cpu->opCode](Value, cpu);
	}
	else // else read was fine, execute instruction, end op
	{
		PollInterrupts(cpu, 3); // Cycle 3
		Operations[cpu->opCode](Value, cpu);
	}
}

// 7 cycles
void absYIndexReadWrite(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2
	cpu->opHighByte = ReadCpu8(cpu->prgCounter++, cpu, 3); // Cycle 3

	cpu->opTemp = cpu->opLowByte;
	cpu->opLowByte += cpu->Y;
	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 4); // Cycle 4

	if(cpu->opLowByte < cpu->opTemp) // If the page was crossed then fix.
	{
		++cpu->opHighByte;
	}

	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 5); // Cycle 5

	cpu->opValue = Operations[cpu->opCode](cpu->opValue, cpu);
	PollInterrupts(cpu, 6); // Cycle 6
	WriteCpu8(cpu->opValue, (cpu->opHighByte << 8) | cpu->opLowByte, cpu, 7); // Cycle 7
}

// 5 cycles
void absYIndexWrite(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2
	cpu->opHighByte = ReadCpu8(cpu->prgCounter++, cpu, 3); // Cycle 3

	cpu->opTemp = cpu->opLowByte;
	cpu->opLowByte += cpu->Y;
	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 4); // Cycle 4

	if(cpu->opLowByte < cpu->opTemp) // If the page was crossed then fix.
	{
		++cpu->opHighByte;
	}

	PollInterrupts(cpu, 4); // Cycle 4

	u8 Value = Operations[cpu->opCode](0, cpu);
	WriteCpu8(Value, (cpu->opHighByte << 8) | cpu->opLowByte, cpu, 5); // Cycle 5
}

///////////////////////////////
//// INDEXED INDIRECT MODE ////
///////////////////////////////

// 6 Cycles
void idxXRead(Cpu *cpu)
{
	cpu->opValue = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opValue += cpu->X;
	cpu->opLowByte = ReadCpu8(cpu->opValue, cpu, 4); // Cycle 4
	u8 Temp = (cpu->opValue + 1) & 0xFF;
	cpu->opHighByte = ReadCpu8(Temp, cpu, 5); // Cycle 5
	PollInterrupts(cpu, 5); // Cycle 5
	u8 Value = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 6); // Cycle 6
	Operations[cpu->opCode](Value, cpu);
}

// 8 Cycles
void idxXReadWrite(Cpu *cpu)
{
	cpu->opValue = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opValue += cpu->X;
	cpu->opLowByte = ReadCpu8(cpu->opValue, cpu, 4); // Cycle 4
	u8 Temp = (cpu->opValue + 1) & 0xFF;
	cpu->opHighByte = ReadCpu8(Temp, cpu, 5); // Cycle 5
	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 6); // Cycle 6
	cpu->opValue = Operations[cpu->opCode](cpu->opValue, cpu);
	PollInterrupts(cpu, 7); // Cycle 7
	WriteCpu8(cpu->opValue, (cpu->opHighByte << 8) | cpu->opLowByte, cpu, 8); // Cycle 8
}

// 6 Cycles
void idxXWrite(Cpu *cpu)
{
	cpu->opValue = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opValue += cpu->X;
	cpu->opLowByte = ReadCpu8(cpu->opValue, cpu, 4); // Cycle 4
	u8 Temp = (cpu->opValue + 1) & 0xFF;
	cpu->opHighByte = ReadCpu8(Temp, cpu, 5); // Cycle 5
	PollInterrupts(cpu, 5); // Cycle 5
	u8 Value = Operations[cpu->opCode](0, cpu);
	WriteCpu8(Value, (cpu->opHighByte << 8) | cpu->opLowByte, cpu, 6); // Cycle 6
}

///////////////////////////////
//// INDIRECT INDEXED MODE ////
///////////////////////////////

// 5 or 6 cycles
void idxYRead(Cpu *cpu)
{
	cpu->opValue = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opLowByte = ReadCpu8(cpu->opValue, cpu, 3); // Cycle 3
	u8 Temp = (cpu->opValue + 1) & 0xFF;
	cpu->opHighByte = ReadCpu8(Temp, cpu, 4); // Cycle 4
	cpu->opTemp = cpu->opLowByte;
	cpu->opLowByte += cpu->Y;
	u8 Value = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 5); // Cycle 5

	if(cpu->opLowByte < cpu->opTemp) // If the page was crossed then fix.
	{
		cpu->opClockTotal += 1;
		++cpu->opHighByte;
		PollInterrupts(cpu, 5); // Cycle 5

		u8 Value = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 6); // Cycle 6
		Operations[cpu->opCode](Value, cpu);
	}
	else
	{
		PollInterrupts(cpu, 4); // Cycle 4
		Operations[cpu->opCode](Value, cpu);
	}

}

// 8 cycles
void idxYReadWrite(Cpu *cpu)
{
	cpu->opValue = ReadCpu8(cpu->prgCounter++, cpu, 2); // Cycle 2

	cpu->opLowByte = ReadCpu8(cpu->opValue, cpu, 3); // Cycle 3
	u8 Temp = (cpu->opValue + 1) & 0xFF;
	cpu->opHighByte = ReadCpu8(Temp, cpu, 4); // Cycle 4
	cpu->opTemp = cpu->opLowByte;
	cpu->opLowByte += cpu->Y;
	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 5); // Cycle 5

	if(cpu->opLowByte < cpu->opTemp) // If the page was crossed then fix.
	{
		++cpu->opHighByte;
	}
	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu, 6); // Cycle 6

	cpu->opValue = Operations[cpu->opCode](cpu->opValue, cpu);
	PollInterrupts(cpu, 7); // Cycle 7
	WriteCpu8(cpu->opValue, (cpu->opHighByte << 8) | cpu->opLowByte, cpu, 8); // Cycle 8
}

// 6 cycles
void idxYWrite(Cpu *cpu)
{
	cpu->opValue = ReadCpu8(cpu->prgCounter++, cpu); // Cycle 2

	cpu->opLowByte = ReadCpu8(cpu->opValue, cpu); // Cycle 3
	u8 Temp = (cpu->opValue + 1) & 0xFF;
	cpu->opHighByte = ReadCpu8(Temp, cpu); // Cycle 4
	cpu->opTemp = cpu->opLowByte;
	cpu->opLowByte += cpu->Y;
	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu); // Cycle 5

	if(cpu->opLowByte < cpu->opTemp) // If the page was crossed then fix.
	{
		++cpu->opHighByte;
	}

	PollInterrupts(cpu); // Cycle 5

	u8 Value = Operations[cpu->opCode](0, cpu); // Cycle 6
	WriteCpu8(Value, (cpu->opHighByte << 8) | cpu->opLowByte, cpu); // Cycle 6
}

///////////////////////////
//// ABSOLUTE INDIRECT ////
///////////////////////////

// 5 cycles
void absIndJmp(Cpu *cpu)
{
	cpu->opLowByte = ReadCpu8(cpu->prgCounter++, cpu); // Cycle 2
	cpu->opHighByte = ReadCpu8(cpu->prgCounter++, cpu); // Cycle 3

	cpu->opValue = ReadCpu8((cpu->opHighByte << 8) | cpu->opLowByte, cpu);  // Cycle 4
	PollInterrupts(cpu); // Cycle 4
	u8 IndLowByte = (u8)(cpu->opLowByte + 1);
	cpu->opHighByte = ReadCpu8((cpu->opHighByte << 8) | IndLowByte, cpu); // Cycle 5
	cpu->opLowByte = cpu->opValue;
	Operations[cpu->opCode](0, cpu); // Cycle 5
}

#define ADDRESS_MODE_COUNT 30

void (*OperationAddressModes[ADDRESS_MODE_COUNT])(Cpu *cpu) =
{
	accumulator, implied, immediate, relative,
	zeroRead, zeroReadWrite, zeroWrite,
	zeroXIndexRead, zeroXIndexReadWrite, zeroXIndexWrite,
	zeroYIndexRead, zeroYIndexReadWrite, zeroYIndexWrite,
	absRead, absReadWrite, absWrite,
	absXIndexRead, absXIndexReadWrite, absXIndexWrite,
	absYIndexRead, absYIndexReadWrite, absYIndexWrite,
	idxXRead, idxXReadWrite, idxXWrite,
	idxYRead, idxYReadWrite, idxYWrite,
	absJmp, absIndJmp
};
