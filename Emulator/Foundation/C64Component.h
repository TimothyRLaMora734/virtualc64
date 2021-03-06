// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _C64_COMPONENT_INC
#define _C64_COMPONENT_INC

#include "HardwareComponent.h"

//
// Forward declarations of all components
//

class C64Memory;
class CPU;
class ProcessorPort;
class VIC;
class CIA1;
class CIA2;
class SIDBridge;
class Keyboard;
class ControlPort;
class ExpansionPort;
class IEC;
class VC1541;
class Datasette;
class Mouse;

//
// Base class for all C64 components
//

class C64Component : public HardwareComponent {

protected:

    C64 &vc64;
    C64Memory &mem;
    CPU &cpu;
    ProcessorPort &pport;
    VIC &vic;
    CIA1 &cia1;
    CIA2 &cia2;
    SIDBridge &sid;
    Keyboard &keyboard;
    ControlPort &port1;
    ControlPort &port2;
    ExpansionPort &expansionport;
    IEC &iec;
    VC1541 &drive1;
    VC1541 &drive2;
    Datasette &datasette;
    Mouse &mouse;
    
    VC1541 *drive[2] = { &drive1, &drive2 };

public:

    C64Component(C64& ref);

private:

    void prefix() override;
};

#endif
