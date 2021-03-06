// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _STARDOS_INC
#define _STARDOS_INC

#include "Cartridge.h"

class StarDos : public Cartridge {
    
    private:
    
    u64 voltage = 5000000;
    u64 latestVoltageUpdate = 0;
    
    public:
    
    StarDos(C64 *c64, C64 &ref) : Cartridge(c64, ref, "StarDos") { };
    CartridgeType getCartridgeType() { return CRT_STARDOS; }
    void reset();
    
    void updateVoltage();
    void charge();
    void discharge();
    void enableROML();
    void disableROML();

    void updatePeekPokeLookupTables(); 
    u8 peekIO1(u16 addr) { charge(); return 0; }
    u8 peekIO2(u16 addr) { discharge(); return 0; }
    void pokeIO1(u16 addr, u8 value) { charge(); }
    void pokeIO2(u16 addr, u8 value) { discharge(); }
    
    bool hasResetButton() { return true; }
};

#endif
