/*
 * (C) 2009 - 2015 Dirk W. Hoffmann. All rights reserved.
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


#ifndef _SNAPSHOT_INC
#define _SNAPSHOT_INC

#include "Container.h"
#include "VIC_globals.h"

// Forward declarations
class C64;

// Snapshot header
typedef struct {
    
    //! @brief    Magic bytes ('V','C','6','4')
    char magic[4];
    
    //! @brief    Version number (V major.minor.subminor)
    uint8_t major;
    uint8_t minor;
    uint8_t subminor;
    
    //! @brief    Screenshot
    struct {
        
        //! @brief    Image width and height
        uint16_t width, height;
        
        //! @brief    Screen buffer data
        uint32_t screen[PAL_RASTERLINES * NTSC_PIXELS];
        
    } screenshot;
    
    //! @brief    Date and time of snapshot creation
    time_t timestamp;
    
    //! @brief    Size of internal state
    uint32_t size;
    
} SnapshotHeader;

/*! @class    Snapshot
 *  @brief    The Snapshot class declares the programmatic interface for a file that contains an emulator snapshot 
 *            (a frozen internal state).
 */
class Snapshot : public Container {
	
private:
	
    //! @brief    Header signature
    static const uint8_t magicBytes[];
    
    //! @brief    Internal state data
    SnapshotHeader *header;
    uint8_t *state;
	
public:

	//! @brief    Constructor
	Snapshot();
	
    //! @brief    Factory method
    static Snapshot *makeSnapshotWithFile(const char *filename);
    
    //! @brief    Factory method
    static Snapshot *makeSnapshotWithBuffer(const uint8_t *buffer, size_t size);

    //! @brief    Factory method
    static Snapshot *makeSnapshotWithC64(C64 *c64);

	//! @brief    Destructor
	~Snapshot();
	
    //! @brief    Frees the allocated memory
    void dealloc();

    //! @brief    Allocates memory for storing internal state
    bool alloc(size_t size);

    //! @brief    Returns true iff buffer contains a snapshot
    static bool isSnapshot(const uint8_t *buffer, size_t length);

    //! @brief    Returns true iff buffer contains a snapshot of a specific version
    static bool isSnapshot(const uint8_t *buffer, size_t length,
                           uint8_t major, uint8_t minor, uint8_t subminor);
 
    //! @brief    Returns true iff buffer contains a snapshot with a supported version number
    static bool isSupportedSnapshot(const uint8_t *buffer, size_t length);

    //! @brief    Returns true iff buffer contains a snapshot with an outdated version number
    static bool isUnsupportedSnapshot(const uint8_t *buffer, size_t length);

    //! @brief    Returns true if path points to a snapshot file
    static bool isSnapshotFile(const char *path);

    //! @brief    Returns true if file points to a snapshot file of a specific version
    static bool isSnapshotFile(const char *path, uint8_t major, uint8_t minor, uint8_t subminor);

    //! @brief    Returns true if file is a snapshot with a supported version number
    static bool isSupportedSnapshotFile(const char *path);

    //! @brief    Returns true if file is a snapshot with an outdated version number
    static bool isUnsupportedSnapshotFile(const char *path);

    //
	// Virtual functions from Container class
    //
    
	bool hasSameType(const char *filename);
	bool readFromBuffer(const uint8_t *buffer, size_t length);
	size_t writeToBuffer(uint8_t *buffer);
    ContainerType type();
	const char *typeAsString();

    //! @brief    Returns size of header
    size_t getHeaderSize() { return sizeof(SnapshotHeader); }

    //! @brief    Returns pointer to header data
    SnapshotHeader *getHeader() { return header; }

    //! @brief    Returns size of core data
    uint32_t getDataSize() { return getHeader()->size; }

    //! @brief    Returns pointer to core data
	uint8_t *getData() { return state; }

	//! @brief    Returns the timestamp
	time_t getTimestamp() { return getHeader()->timestamp; }

	//! @brief    Sets the timestamp
	void setTimestamp(time_t value) { getHeader()->timestamp = value; }
	
	//! Returns true, if snapshot does not contain data yet
	bool isEmpty() { return state == NULL; }
	
	//! Return screen buffer
	unsigned char *getImageData() { return (unsigned char *)(getHeader()->screenshot.screen); }

    //! Return image width
    unsigned getImageWidth() { return getHeader()->screenshot.width; }

    //! Return image height
    unsigned getImageHeight() { return getHeader()->screenshot.height; }

    //! Take screenshot
    void takeScreenshot(uint32_t *buf, bool pal);

};

#endif
	
