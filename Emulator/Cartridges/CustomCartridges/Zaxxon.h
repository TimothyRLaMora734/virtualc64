// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _ZAXXON_INC
#define _ZAXXON_INC

#include "Cartridge.h"

class Zaxxon : public Cartridge {
    
public:
    using Cartridge::Cartridge;
    CartridgeType getCartridgeType() { return CRT_ZAXXON; }

    void reset();
    u8 peekRomL(u16 addr);
    u8 spypeekRomL(u16 addr);
};

#endif
