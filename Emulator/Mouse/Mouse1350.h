// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef MOUSE1350_H
#define MOUSE1350_H

#include "C64Component.h"

class Mouse1350 : public C64Component {
    
private:
    
    // Mouse position
    i64 mouseX;
    i64 mouseY;
    
    // Mouse button states
    bool leftButton;
    bool rightButton;
    
    // Dividers applied to raw coordinates in setXY()
    int dividerX = 64;
    int dividerY = 64;
    
    // Latched mouse positions
    i64 latchedX[3];
    i64 latchedY[3];
    
    // Control port bits
    u8 controlPort;
    
    
    //
    // Constructing and serializing
    //
    
public:
    
    Mouse1350(C64 &ref);
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void reset() override;
    
    
    //
    // Using the device
    //
    
public:
    
    // Updates the button state
    void setLeftMouseButton(bool value) { leftButton = value; }
    void setRightMouseButton(bool value) { rightButton = value; }

    // Returns the pot bits as set by the mouse
    u8 readPotX();
    u8 readPotY();
    
    // Returns the control port bits triggered by the mouse
    u8 readControlPort();
    
    // Execution function (Translates movement deltas into joystick events)
    void execute(i64 targetX, i64 targetY);
};

#endif