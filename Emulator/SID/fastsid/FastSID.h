/*
 * This file belongs to the FastSID implementation of VirtualC64,
 * an adaption of the code used in VICE 3.1, the Versatile Commodore Emulator.
 *
 * Original code written by
 *  Teemu Rantanen <tvr@cs.hut.fi>
 *
 * Adapted for VirtualC64 by
 *  Dirk Hoffmann
 */
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef _FASTSID_INC
#define _FASTSID_INC

#include "C64Component.h"
#include "FastVoice.h"


//! The virtual sound interface device (SID)
/*! SID is the sound chip of the Commodore 64.
	The SID chip occupied the memory mapped I/O space from address 0xD400 to 0xD7FF. 
*/
class FastSID : public C64Component {

public:
    
    //! Pointer to bridge object
    class SIDBridge *bridge;
    
    //! @brief   SID registers
    u8 sidreg[32];
    
    //! @brief   Internal constant used for sample rate dependent calculations
    u32 speed1;
    
private:
    
    //! @brief   The three SID voices
    FastVoice voice[3];
        
    //! @brief   Chip model.
    SIDModel model = MOS_6581;
    
    //! @brief   Current CPU frequency
    u32 cpuFrequency = PAL_CLOCK_FREQUENCY;
    
    //! @brief   Sample rate (44.1 kHz per default)
    u32 sampleRate = 44100;
    
    //! @brief    Ratio between sample rate and cpu frequency
    double   samplesPerCycle;
    
    //! @brief   Stores for how many cycles FastSID was executed so far
    u64 executedCycles;

    //! @brief   Stores how many sound samples were computed so far
    u64 computedSamples;

    //! @brief   Switches filter emulation on or off.
    bool emulateFilter = true;
    
    //! @brief   Last value on the data bus
    u8 latchedDataBus;
    
public:
    
    //! @brief   ADSR counter step lookup table
    i32 adrs[16];
    
    //! @brief   Sustain comparison values loopup table
    u32 sz[16];
    
private:
    
    //! @brief   Low pass filter lookup table
    /*! @details Needs to be updated when the sample rate changes
     */
    float lowPassParam[0x800];

    //! @brief   Band pass filter lookup table
    /*! @details Needs to be updated when the sample rate changes
     */
    float bandPassParam[0x800];
    
    //! @brief   Filter resonance lookup table
    /*! @details Needs to be updated when the sample rate changes
     */
    float filterResTable[16];
    
    //! @brief   Amplifier lookup table
    signed char ampMod1x8[256];
    
public:
    
	// Constructor
	FastSID(C64 &ref);
	
	// Destructor
	~FastSID();
	
    // Method from HardwareComponent
    void reset();

    // Sets the clock frequency
    void setClockFrequency(u32 frequency);

    // Dump internal state to console
    void dump();
    
    // Gathers all values that are displayed in the debugger
    SIDInfo getInfo();
    
    // Gathers all debug information for a specific voice
    VoiceInfo getVoiceInfo(unsigned voice);
    
    // Special peek function for the I/O memory range
    u8 peek(u16 addr);
    
    // Special poke function for the I/O memory range.
    void poke(u16 addr, u8 value);
    
    /* Execute SID
     * Runs reSID for the specified amount of CPU cycles and writes the
     * generated sound samples into the internal ring buffer.
     */
    void execute(u64 cycles);
    
    // Computes a single sound sample
    i16 calculateSingleSample();
    
    
    //
    // Configuring the device
    //
    
    // Returns the chip model
    SIDModel getModel() { return model; }
    
    // Sets the chip model
    void setModel(SIDModel m);
    
    // Returns the clock frequency
    u32 getClockFrequency() { return cpuFrequency; }
    
    // Returns the sample rate
    u32 getSampleRate() { return sampleRate; }
    
    // Sets the sample rate
    void setSampleRate(u32 rate);
    
    // Returns true iff audio filters should be emulated.
    bool getAudioFilter() { return emulateFilter; }
    
    // Enable or disable audio filter emulation
    void setAudioFilter(bool value) { emulateFilter = value; }
    
private:
    
    // Initializes SID
    void init(int sampleRate, int cycles_per_sec);
    
    // Initializes filter lookup tables
    void initFilter(int sampleRate);
    
    
    //
    // Accessing device properties
    //
    
    // Returns the currently set SID volume
    u8 sidVolume() { return sidreg[0x18] & 0x0F; }
    
    // Returns true iff voice 3 is disconnected from the audio output
    /* Setting voice 3 to bypass the filter (FILT3 = 0) and setting bit 7 in
     * the Mod/Vol register to one prevents voice 3 from reaching the audio
     * output.
     */
    bool voiceThreeDisconnected() { return filterOff(2) && (sidreg[0x18] & 0x80); }
    
    // Filter related configuration items
    
    // Returns the filter cutoff frequency (11 bit value)
    u16 filterCutoff() { return (sidreg[0x16] << 3) | (sidreg[0x15] & 0x07); }

    // Returns the filter resonance (4 bit value)
    u8 filterResonance() { return sidreg[0x17] >> 4; }

    // Returns true iff the specified voice schould be filtered
    bool filterOn(unsigned voice) { return GET_BIT(sidreg[0x17], voice) != 0; }

    // Returns true iff the specified voice schould not be filtered
    bool filterOff(unsigned voice) { return GET_BIT(sidreg[0x17], voice) == 0; }

    // Returns true iff the external filter bit is set
    bool filterExtBit() { return GET_BIT(sidreg[0x17], 7) != 0; }
    
    // Returns the currently set filter type
    u8 filterType() { return sidreg[0x18] & 0x70; }
    
    
    // Updates internal data structures
    // This method is called on each filter related register change
    void updateInternals();
};

#endif
