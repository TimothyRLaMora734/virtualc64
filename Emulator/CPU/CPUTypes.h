// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef CPU_TYPES_H
#define CPU_TYPES_H

#include <stdint.h>

//! @brief    Processor model
typedef enum {
    MOS_6510 = 0,
    MOS_6502 = 1
} CPUModel;

//! @brief    Addressing mode
typedef enum {
    ADDR_IMPLIED,
    ADDR_ACCUMULATOR,
    ADDR_IMMEDIATE,
    ADDR_ZERO_PAGE,
    ADDR_ZERO_PAGE_X,
    ADDR_ZERO_PAGE_Y,
    ADDR_ABSOLUTE,
    ADDR_ABSOLUTE_X,
    ADDR_ABSOLUTE_Y,
    ADDR_INDIRECT_X,
    ADDR_INDIRECT_Y,
    ADDR_RELATIVE,
    ADDR_DIRECT,
    ADDR_INDIRECT
} AddressingMode;

/*! @brief    Breakpoint type
 *  @details  Each memory call is marked with a breakpoint tag. Originally,
 *            each cell is tagged with NO_BREAKPOINT which has no effect. CPU
 *            execution will stop if the memory cell is tagged with one of the
 *            following breakpoint types:
 *            HARD_BREAKPOINT : Execution is halted.
 *            SOFT_BREAKPOINT : Execution is halted and the tag is deleted.
 */
typedef enum {
    NO_BREAKPOINT   = 0x00,
    HARD_BREAKPOINT = 0x01,
    SOFT_BREAKPOINT = 0x02
} Breakpoint;


/*! @brief    Error state of the virtual CPU
 *  @details  CPU_OK indicates normal operation. When a (soft or hard)
 *            breakpoint is reached, state CPU_BREAKPOINT_REACHED is entered.
 *            CPU_ILLEGAL_INSTRUCTION is set when an opcode is not understood
 *            by the CPU. Once the CPU enters a different state than CPU_OK,
 *            the execution thread is terminated.
 */
typedef enum {
    CPU_OK = 0,
    CPU_SOFT_BREAKPOINT_REACHED,
    CPU_HARD_BREAKPOINT_REACHED,
    CPU_ILLEGAL_INSTRUCTION
} ErrorState;

/*! @brief    CPU info
 *  @details  Used by getInfo() to collect debug information
 */
typedef struct {
    u64 cycle;
    u16 pc;
    u8 a;
    u8 x;
    u8 y;
    u8 sp;
    bool nFlag;
    bool vFlag;
    bool bFlag;
    bool dFlag;
    bool iFlag;
    bool zFlag;
    bool cFlag;
} CPUInfo;

//! @brief    Recorded instruction
typedef struct {
    u64 cycle;
    u16 pc;
    u8 byte1;
    u8 byte2;
    u8 byte3;
    u8 a;
    u8 x;
    u8 y;
    u8 sp;
    u8 flags;
} RecordedInstruction;

//! @brief    Disassembled instruction
typedef struct {
    u64 cycle;
    u16 addr; 
    u8 size;
    char byte1[4];
    char byte2[4];
    char byte3[4];
    char pc[6];
    char a[4];
    char x[4];
    char y[4];
    char sp[4];
    char flags[9];
    char command[16];
} DisassembledInstruction;

#endif
