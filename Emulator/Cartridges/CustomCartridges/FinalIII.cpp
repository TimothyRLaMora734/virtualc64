// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "C64.h"

void
FinalIII::reset()
{
    CartridgeWithRegister::reset();
    freeezeButtonIsPressed = false;
    qD = true;
    bankIn(0);
}

void
FinalIII::resetCartConfig()
{
    c64->expansionport.setCartridgeMode(CRT_16K);
}

u8
FinalIII::peekIO1(u16 addr)
{
    // I/O space 1 mirrors $1E00 to $1EFF from ROML
    return peekRomL(addr & 0x1FFF);
}

u8
FinalIII::peekIO2(u16 addr)
{
    // I/O space 2 space mirrors $1F00 to $1FFF from ROML
    return peekRomL(addr & 0x1FFF);
}

void
FinalIII::pokeIO2(u16 addr, u8 value) {
    
    // The control register is mapped to address 0xFF in I/O space 2.
    if (addr == 0xDFFF && writeEnabled()) {
        setControlReg(value);
    }
}

void
FinalIII::nmiDidTrigger()
{
    if (freeezeButtonIsPressed) {
        debug(CRT_DEBUG, "NMI while freeze button is pressed.\n");
        
        // After the NMI has been processed by the CPU, the cartridge's counter
        // has reached a value that overflows qD to 0. This has two side
        // effects. First, the Game line switches to 0. Second, because qD is
        // also connectec to the counter's enable pin, the counter freezes. This
        // keeps qD low until the freeze button is released by the user.
        
        qD = false;
        updateGame();
    }
}

const char *
FinalIII::getButtonTitle(unsigned nr)
{
    return (nr == 1) ? "Freeze" : (nr == 2) ? "Reset" : NULL;
}

void
FinalIII::pressButton(unsigned nr)
{
    assert(nr <= numButtons());
    debug(CRT_DEBUG, "Pressing %s button.\n", getButtonTitle(nr));
    
    c64->suspend();
    
    switch (nr) {
            
        case 1: // Freeze
            
            freeezeButtonIsPressed = true;
            updateNMI();
            break;
            
        case 2: // Reset
            
            resetWithoutDeletingRam();
            break;
    }
    
    c64->resume();
}

void
FinalIII::releaseButton(unsigned nr)
{
    assert(nr <= numButtons());
    debug(CRT_DEBUG, "Releasing %s button.\n", getButtonTitle(nr));
    
    c64->suspend();
    
    switch (nr) {
            
        case 1: // Freeze
            
            freeezeButtonIsPressed = false;
            qD = true;
            updateNMI();
            updateGame();
            break;
    }
    
    c64->resume();
}

void
FinalIII::setControlReg(u8 value)
{
    control = value;
    
    // Update external lines
    updateNMI();
    updateGame();
    c64->expansionport.setExromLine(exrom());
    
    // Switch memory bank
    bankIn(control & 0x03);
    
}

bool
FinalIII::writeEnabled()
{
    return !hidden() || freeezeButtonIsPressed;
}

void
FinalIII::updateNMI()
{
    if (nmi() && !freeezeButtonIsPressed) {
        c64->cpu.releaseNmiLine(CPU::INTSRC_EXPANSION);
    } else {
        c64->cpu.pullDownNmiLine(CPU::INTSRC_EXPANSION);
    }
}

void
FinalIII::updateGame()
{
    c64->expansionport.setGameLine(game() && qD);
}
