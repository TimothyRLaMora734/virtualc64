/*!
 * @file        FlashRom.cpp
 * @author      Dirk W. Hoffmann, www.dirkwhoffmann.de
 * @copyright   Dirk W. Hoffmann, all rights reserved.
 */
/* This program is free software; you can redistribute it and/or modify
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

#include "FlashRom.h"

const char *
FlashRom::getStateAsString(FlashRomState state)
{
    switch(state) {
        case FLASH_READ:return "FLASH_READ";
        case FLASH_MAGIC_1: return "FLASH_MAGIC_1";
        case FLASH_MAGIC_2: return "FLASH_MAGIC_2";
        case FLASH_AUTOSELECT: return "FLASH_AUTOSELECT";
        case FLASH_BYTE_PROGRAM: return "FLASH_BYTE_PROGRAM";
        case FLASH_BYTE_PROGRAM_ERROR: return "FLASH_BYTE_PROGRAM_ERROR";
        case FLASH_ERASE_MAGIC_1: return "FLASH_ERASE_MAGIC_1";
        case FLASH_ERASE_MAGIC_2: return "FLASH_ERASE_MAGIC_2";
        case FLASH_ERASE_SELECT: return "FLASH_ERASE_SELECT";
        case FLASH_CHIP_ERASE: return "FLASH_CHIP_ERASE";
        case FLASH_SECTOR_ERASE: return "FLASH_SECTOR_ERASE";
        case FLASH_SECTOR_ERASE_TIMEOUT: return "FLASH_SECTOR_ERASE_TIMEOUT";
        case FLASH_SECTOR_ERASE_SUSPEND: return "FLASH_SECTOR_ERASE_SUSPEND";
        default:
        assert(false);
    }
}

FlashRom::FlashRom()
{
    setDescription("FlashRom");
    debug(3, "  Creating FlashRom at address %p...\n", this);
    
    state = FLASH_READ;
    baseState = FLASH_READ;
    
    numSectors = 8;
    sectorSize = 0x10000; // 64 KB
    size = 0x80000;       // 512 KB
    
    rom = new uint8_t[size];
    memset(rom, 0xFF, size);
    
    // Register snapshot items
    SnapshotItem items[] = {
        { &state,             sizeof(state),                KEEP_ON_RESET },
        { &baseState,         sizeof(baseState),            KEEP_ON_RESET },
        { rom,                size,                         KEEP_ON_RESET },
        { NULL,               0,                            0 }};
    
    registerSnapshotItems(items, sizeof(items));
}

FlashRom::~FlashRom()
{
    debug(3, "  Releasing FlashRom ...\n");
    delete[] rom;
}

void
FlashRom::loadBank(unsigned bank, uint8_t *data)
{
    assert(data != NULL);
    memcpy(rom + bank * 0x2000, data, 0x2000);
}

void
FlashRom::reset()
{
    state = FLASH_READ;
    baseState = FLASH_READ;
}

uint8_t
FlashRom::peek(uint32_t addr)
{
    assert(addr < size);
    
    uint8_t result;
    
    switch (state) {
        
        case FLASH_AUTOSELECT:
        
        // TODO
        result = rom[addr];
        break;
        
        case FLASH_BYTE_PROGRAM_ERROR:
        
        // TODO
        result = rom[addr];
        break;
        
        case FLASH_SECTOR_ERASE_SUSPEND:
        
        // TODO
        result = rom[addr];
        break;
        
        case FLASH_CHIP_ERASE:
        
        // TODO
        result = rom[addr];
        break;
        
        case FLASH_SECTOR_ERASE:
        
        // TODO
        result = rom[addr];
        break;
        
        case FLASH_SECTOR_ERASE_TIMEOUT:
        
        // TODO
        result = rom[addr];
        break;
        
        default:
        
        // TODO
        result = rom[addr];
        break;
    }
    
    return result;
}

void
FlashRom::poke(uint32_t addr, uint8_t value)
{
    assert(addr < size);
    
    switch (state) {
        
        case FLASH_READ:
        
        if (addr == 0x5555 && value == 0xAA) {
            
            state = FLASH_MAGIC_1;
            return;
        }
        
        return;
        
        case FLASH_MAGIC_1:
        
        if (addr == 0x2AAA && value == 0x55) {
            
            state = FLASH_MAGIC_2;
            return;
        }
        
        state = baseState;
        return;
        
        case FLASH_MAGIC_2:
        
        if (addr == 0x5555) {
            
            switch(value) {
                
                case 0xF0:
                
                state = FLASH_READ;
                baseState = FLASH_READ;
                return;
                
                case 0x90:
                
                state = FLASH_AUTOSELECT;
                baseState = FLASH_AUTOSELECT;
                return;
                
                case 0xA0:
                state = FLASH_BYTE_PROGRAM;
                return;
                
                case 0x80:
                state = FLASH_ERASE_MAGIC_1;
                return;
            }
        }
        
        state = baseState;
        break;
        
        case FLASH_BYTE_PROGRAM:
        
        if (!doByteProgram(addr, value)) {
            
            state = FLASH_BYTE_PROGRAM_ERROR;
            return;
        }
        
        state = baseState;
        return;
        
        case FLASH_ERASE_MAGIC_1:
        
        // TODO
        break;
        
        case FLASH_ERASE_MAGIC_2:
        
        // TODO
        break;
        
        case FLASH_ERASE_SELECT:
        
        // TODO
        break;
        
        case FLASH_SECTOR_ERASE_TIMEOUT:
        
        // TODO
        break;
        
        case FLASH_SECTOR_ERASE:
        
        // TODO
        break;
        
        case FLASH_SECTOR_ERASE_SUSPEND:
        
        // TODO
        break;
        
        case FLASH_BYTE_PROGRAM_ERROR:
        case FLASH_AUTOSELECT:
        
        if (addr == 0x5555 && value == 0xAA) {
            
            state = FLASH_MAGIC_1;
            return;
        }
        if (value == 0xF0) {
            
            state = FLASH_READ;
            baseState = FLASH_READ;
            return;
        }
        return;
        
        case FLASH_CHIP_ERASE:
        default:
        
        // TODO
        break;
    }
}

bool
FlashRom::doByteProgram(uint32_t addr, uint8_t value)
{
    assert(addr < size);
    
    rom[addr] &= value;
    return rom[addr] == value;
}

void
FlashRom::doChipErase() {
    
    debug("Erasing chip ...\n");
    memset(rom, 0xFF, size);
}

void
FlashRom::doSectorErase(uint32_t addr)
{
    assert(addr < size);
    
    debug("Erasing sector %d ... %d\n", addr >> 4);
    memset(rom + (addr & 0x0000), 0xFF, sectorSize);
}
