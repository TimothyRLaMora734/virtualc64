// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _EXPERT_INC
#define _EXPERT_INC

#include "Cartridge.h"

class Expert : public Cartridge {
    
    // On-board flipflop
    bool active;
    
public:
    
    Expert(C64 *c64, C64 &ref);
    CartridgeType getCartridgeType() { return CRT_EXPERT; }
    
    //
    //! @functiongroup Methods from HardwareComponent
    //
    
    void reset();
    void dump();
    size_t stateSize();
    void didLoadFromBuffer(u8 **buffer);
    void didSaveToBuffer(u8 **buffer);
    
    //
    //! @functiongroup Methods from Cartridge
    //
    
    void loadChip(unsigned nr, CRTFile *c);
    
    unsigned numButtons() { return 2; }
    const char *getButtonTitle(unsigned nr);
    void pressButton(unsigned nr);
    
    bool hasSwitch() { return true; }
    const char *getSwitchDescription(i8 pos);
    bool switchInPrgPosition() { return switchIsLeft(); }
    bool switchInOffPosition() { return switchIsNeutral(); }
    bool switchInOnPosition() { return switchIsRight(); }

    void updatePeekPokeLookupTables();
    u8 peek(u16 addr);
    u8 peekIO1(u16 addr);
    u8 spypeekIO1(u16 addr) { return 0; }
    void poke(u16 addr, u8 value);
    void pokeIO1(u16 addr, u8 value);
    
    void nmiWillTrigger();
    
    //! @brief    Returns true if cartridge RAM is visible
    bool cartridgeRamIsVisible(u16 addr);    

    //! @brief    Returns true if cartridge RAM is write enabled
    bool cartridgeRamIsWritable(u16 addr);
};


#endif
