/*
 * (C) 2006 Dirk W. Hoffmann. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "C64.h"

CPU::CPU()
{	
	setDescription("CPU");
	debug(3, "  Creating CPU at address %p...\n", this);
	
    // Chip model
    chipModel = MOS_6510;

	// Establish callback for each instruction
	registerInstructions();
		
	// Clear all breakpoint tags
	for (int i = 0; i <  65536; i++) {
		breakpoint[i] = NO_BREAKPOINT;	
	}
    
    // Register snapshot items
    SnapshotItem items[] = {
        
        // Lifetime items
        { &chipModel,               sizeof(chipModel), KEEP_ON_RESET },

         // Internal state
        { &A,                       sizeof(A),                      CLEAR_ON_RESET },
        { &X,                       sizeof(X),                      CLEAR_ON_RESET },
        { &Y,                       sizeof(Y),                      CLEAR_ON_RESET },
        { &PC,                      sizeof(PC),                     CLEAR_ON_RESET },
        { &PC_at_cycle_0,           sizeof(PC_at_cycle_0),          CLEAR_ON_RESET },
        { &SP,                      sizeof(SP),                     CLEAR_ON_RESET },
        { &N,                       sizeof(N),                      CLEAR_ON_RESET },
        { &V,                       sizeof(V),                      CLEAR_ON_RESET },
        { &B,                       sizeof(B),                      CLEAR_ON_RESET },
        { &D,                       sizeof(D),                      CLEAR_ON_RESET },
        { &I,                       sizeof(I),                      CLEAR_ON_RESET },
        { &Z,                       sizeof(Z),                      CLEAR_ON_RESET },
        { &C,                       sizeof(C),                      CLEAR_ON_RESET },
        { &opcode,                  sizeof(opcode),                 CLEAR_ON_RESET },
        { &addr_lo,                 sizeof(addr_lo),                CLEAR_ON_RESET },
        { &addr_hi,                 sizeof(addr_hi),                CLEAR_ON_RESET },
        { &ptr,                     sizeof(ptr),                    CLEAR_ON_RESET },
        { &pc_lo,                   sizeof(pc_lo),                  CLEAR_ON_RESET },
        { &pc_hi,                   sizeof(pc_hi),                  CLEAR_ON_RESET },
        { &overflow,                sizeof(overflow),               CLEAR_ON_RESET },
        { &data,                    sizeof(data),                   CLEAR_ON_RESET },
        { &rdyLine,                 sizeof(rdyLine),                CLEAR_ON_RESET },
        { &irqLine,                 sizeof(irqLine),                CLEAR_ON_RESET },
        { &nmiLine,                 sizeof(nmiLine),                CLEAR_ON_RESET },
        { &nmiEdge,                 sizeof(nmiEdge),                CLEAR_ON_RESET },
        { &nextPossibleIrqCycle,    sizeof(nextPossibleIrqCycle),   CLEAR_ON_RESET },
        { &nextPossibleNmiCycle,    sizeof(nextPossibleNmiCycle),   CLEAR_ON_RESET },
        { &errorState,              sizeof(errorState),             CLEAR_ON_RESET },
        { &callStack,               sizeof(callStack),              CLEAR_ON_RESET | WORD_FORMAT },
        { &callStackPointer,        sizeof(callStackPointer),       CLEAR_ON_RESET },
        { &oldI,                    sizeof(oldI),                   CLEAR_ON_RESET },
        { NULL,                     0,                              0 }};
    
    registerSnapshotItems(items, sizeof(items));
}

CPU::~CPU()
{
	debug(3, "  Releasing CPU...\n");
}

void
CPU::reset()
{
    VirtualComponent::reset();
    
    B = 1;
	rdyLine = true;
	next = &CPU::fetch;
}

size_t
CPU::stateSize()
{
    return VirtualComponent::stateSize() + 2;
}

void 
CPU::loadFromBuffer(uint8_t **buffer) 
{
    VirtualComponent::loadFromBuffer(buffer);
	next = CPU::callbacks[read16(buffer)];
}

void
CPU::saveToBuffer(uint8_t **buffer) 
{
    VirtualComponent::saveToBuffer(buffer);
	for (uint16_t i = 0;; i++) {
		if (callbacks[i] == NULL) {
			panic("ERROR while saving state: Callback pointer not found!\n");
		}
		if (callbacks[i] == next) {
			write16(buffer, i);
			break;
		}
	}
}

void 
CPU::dumpState()
{    
	msg("CPU:\n");
	msg("----\n\n");
    msg("%s\n", disassemble());
	msg("      Rdy line : %s\n", rdyLine ? "high" : "low");
	msg("      Irq line : %02X\n", irqLine);
	msg("      Nmi line : %02X %s\n", nmiLine, nmiEdge ? "(negative edge)" : "");
	msg(" no IRQ before : %ull\n", nextPossibleIrqCycle);
	msg(" no NMI before : %ull\n", nextPossibleNmiCycle);
	msg("   IRQ routine : %02X%02X\n", mem->peek(0xFFFF), mem->peek(0xFFFE));
	msg("   NMI routine : %02X%02X\n", mem->peek(0xFFFB), mem->peek(0xFFFA));	
	msg("\n");
    
    c64->processorPort.dumpState();
}

void
CPU::pullDownIrqLine(uint8_t source)
{
	assert(source != 0);
    
	if (irqLine == 0) {
		nextPossibleIrqCycle = c64->getCycles() + 2;
	}
	irqLine |= source;
}

bool 
CPU::IRQLineRaisedLongEnough() 
{ 
	return c64->getCycles() >= nextPossibleIrqCycle;
}

bool
CPU::IRQsAreBlocked() { 
	bool result;
	if (opcode == 0x78 /* SEI */ || opcode == 0x58 /* CLI */)
		result = oldI;
	else
		result = I;

	oldI = I;
	return result;
}

void 
CPU::setNMILine(uint8_t bit) // TODO: RENAME TO pullDownNmiLine
{ 
	assert(bit != 0);

    if (!nmiLine) {
        nmiEdge = true;
        nextPossibleNmiCycle = c64->getCycles() + 2;
    }
    
	nmiLine |= bit; 
}

bool
CPU::NMILineRaisedLongEnough()
{ 
	return c64->getCycles() >= nextPossibleNmiCycle;
}


// Instruction set
const char 
*CPU::getMnemonic(uint8_t opcode)
{
	return mnemonic[opcode];
}

AddressingMode
CPU::getAddressingMode(uint8_t opcode)
{
	return addressingMode[opcode];
}

int 
CPU::getLengthOfInstruction(uint8_t opcode)
{
	switch(addressingMode[opcode]) {
		case ADDR_IMPLIED:			
		case ADDR_ACCUMULATOR:
			return 1;
		case ADDR_IMMEDIATE:
		case ADDR_ZERO_PAGE:
		case ADDR_ZERO_PAGE_X:
		case ADDR_ZERO_PAGE_Y:
		case ADDR_INDIRECT_X:
		case ADDR_INDIRECT_Y:
		case ADDR_RELATIVE:
			return 2;
		case ADDR_ABSOLUTE:
		case ADDR_ABSOLUTE_X:
		case ADDR_ABSOLUTE_Y:
		case ADDR_DIRECT:
		case ADDR_INDIRECT:
			return 3;
	}
	return 1;
}

char *
CPU::disassemble()
{
	char buf[64], msg[128];
	int i, op;
	
	uint16_t pc = PC_at_cycle_0;
	uint8_t opcode = mem->peek(pc);	
	
	strcpy(msg, "");
	
	// Program counter
	sprintf(buf, "%04X: ", pc);
	strcat(msg, buf);
	
	// Hex dump
	for (i = 0; i < 3; i++) {
		if (i < getLengthOfInstruction(opcode)) {
			sprintf(buf, "%02X ", mem->peek(pc+i));
			strcat(msg, buf);
		} else {
			sprintf(buf, "   ");
			strcat(msg, buf);
		}
	}
	
	// Register
	sprintf(buf, "  %02X %02X %02X %02X ", A, X, Y, SP);
	strcat(msg, buf);
	
	// Flags
	sprintf(buf, "%c%c%c%c%c%c%c%c ",
			N ? 'N' : 'n',
			V ? 'V' : 'v',
			'-',
			B ? 'B' : 'b',
			D ? 'D' : 'd',
			I ? 'I' : 'i',
			Z ? 'Z' : 'z',
			C ? 'C' : 'c');
	strcat(msg, buf);
	
	// Mnemonic
	sprintf(buf, "%s ", getMnemonic(opcode));
	strcat(msg, buf);
	
	// Get operand as number
	switch (addressingMode[opcode]) {
		case ADDR_IMMEDIATE:
		case ADDR_ZERO_PAGE:
		case ADDR_ZERO_PAGE_X:
		case ADDR_ZERO_PAGE_Y:
		case ADDR_INDIRECT_X:
		case ADDR_INDIRECT_Y:
			op = mem->peek(pc+1);
			break;
		case ADDR_DIRECT:			
		case ADDR_INDIRECT:
		case ADDR_ABSOLUTE:
		case ADDR_ABSOLUTE_X:
		case ADDR_ABSOLUTE_Y:
			op = mem->peekWord(pc+1);
			break;
		case ADDR_RELATIVE:
			op = pc + 2 + (int8_t)mem->peek(pc+1);
			break;
		default:
			op = -1;
	}
	
	// Format operand
	switch (addressingMode[opcode]) {
		case ADDR_IMPLIED:
		case ADDR_ACCUMULATOR:
			sprintf(buf, " ");
			break;
		case ADDR_IMMEDIATE:					
			sprintf(buf, "#%02X", op);
			break;
		case ADDR_ZERO_PAGE:	
			sprintf(buf, "%02X", op);
			break;
		case ADDR_ZERO_PAGE_X:	
			sprintf(buf, "%02X,X", op);
			break;
		case ADDR_ZERO_PAGE_Y:	
			sprintf(buf, "%02X,Y", op);
			break;
		case ADDR_ABSOLUTE:	
		case ADDR_DIRECT:
			sprintf(buf, "%04X", op);
			break;
		case ADDR_ABSOLUTE_X:	
			sprintf(buf, "%04X,X", op);
			break;
		case ADDR_ABSOLUTE_Y:	
			sprintf(buf, "%04X,Y", op);
			break;
		case ADDR_INDIRECT:	
			sprintf(buf, "(%04X)", op);
			break;
		case ADDR_INDIRECT_X:	
			sprintf(buf, "(%04X,X)", op);
			break;
		case ADDR_INDIRECT_Y:	
			sprintf(buf, "(%04X),Y", op);
			break;
		case ADDR_RELATIVE:
			sprintf(buf, "%04X", op);
			break;
		default:
			sprintf(buf, "???");
	}
	strcat(msg, buf);
	return strdup(msg);
}

void 
CPU::setErrorState(ErrorState state)
{
	if (errorState == state)
        return;

    errorState = state;
    
    switch (errorState) {
        case CPU_OK:
            c64->putMessage(MSG_CPU_OK);
            return;
        case CPU_SOFT_BREAKPOINT_REACHED:
            c64->putMessage(MSG_CPU_SOFT_BREAKPOINT_REACHED);
            return; 
        case CPU_HARD_BREAKPOINT_REACHED:
            c64->putMessage(MSG_CPU_HARD_BREAKPOINT_REACHED);
            return;
        case CPU_ILLEGAL_INSTRUCTION:
            c64->putMessage(MSG_CPU_ILLEGAL_INSTRUCTION);
            return;
        default:
            assert(false);
    }
}


