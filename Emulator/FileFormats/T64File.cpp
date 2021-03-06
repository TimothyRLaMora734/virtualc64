// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "TAPFile.h"
#include "T64File.h"

/* "Anmerkung: Der String muß nicht wortwörtlich so vorhanden sein. Man sollte nach den
 *  Substrings 'C64' und 'tape' suchen." [Power64 doc]
 */
const u8 T64File::magicBytes[] = { 0x43, 0x36, 0x34 };

bool
T64File::isT64Buffer(const u8 *buffer, size_t length)
{
    if (length < 0x40)
        return false;
    
    if (TAPFile::isTAPBuffer(buffer, length)) // Note: TAP files have a very similar header
        return false;
    
    return matchingBufferHeader(buffer, magicBytes, sizeof(magicBytes));
}

bool
T64File::isT64File(const char *path)
{
    assert(path != NULL);
    
    if (!checkFileSuffix(path, ".T64") && !checkFileSuffix(path, ".t64"))
        return false;
    
    if (TAPFile::isTAPFile(path)) // Note: TAP files have a very similar header
        return false;
    
    if (!checkFileSize(path, 0x40, -1))
        return false;
    
    if (!matchingFileHeader(path, magicBytes, sizeof(magicBytes)))
        return false;
    
    return true;
}

T64File::T64File()
{
    setDescription("T64Archive");
}

T64File *
T64File::makeWithBuffer(const u8 *buffer, size_t length)
{
    T64File *archive = new T64File();
    
    if (!archive->readFromBuffer(buffer, length)) {
        delete archive;
        return NULL;
    }
    
    return archive;
}

T64File *
T64File::makeWithFile(const char *path)
{
    T64File *archive = new T64File();
    
    if (!archive->readFromFile(path)) {
        delete archive;
        return NULL;
    }
    
    return archive;
}

T64File *
T64File::makeT64ArchiveWithAnyArchive(AnyArchive *otherArchive)
{
    if (otherArchive == NULL)
        return NULL;
    
    T64File *archive  = new T64File();
    
    // Determine file size and allocate memory
    unsigned currentFiles = otherArchive->numberOfItems();
    unsigned maxFiles = (currentFiles < 30) ? 30 : currentFiles;
    archive->size = 64 /* header */ + maxFiles * 32 /* tape entries */;
    
    for (unsigned i = 0; i < currentFiles; i++) {
        archive->selectItem(i);
        archive->size += otherArchive->getSizeOfItem();
    }

    if ((archive->data = new u8[archive->size]) == NULL) {
        archive->warn("Failed to allocate %d bytes of memory\n", archive->size);
        delete archive;
        return NULL;
    }
    
    // Magic bytes (32 bytes)
    u8 *ptr = archive->data;
    strncpy((char *)ptr, "C64 tape image file", 32);
    ptr += 32;
    
    // Version (2 bytes)
    *ptr++ = 0x00;
    *ptr++ = 0x01;
    
    // Max files (2 bytes)
    *ptr++ = LO_BYTE(maxFiles);
    *ptr++ = HI_BYTE(maxFiles);
    
    // Current files (2 bytes)
    *ptr++ = LO_BYTE(currentFiles);
    *ptr++ = HI_BYTE(currentFiles);
    
    // Reserved (2 bytes)
    *ptr++ = 0x00;
    *ptr++ = 0x00;
    
    // User description (24 bytes)
    strncpy((char *)ptr, (char *)otherArchive->getName(), 24);
    for (unsigned i = 0; i < 24; i++, ptr++)
        *ptr = ascii2pet(*ptr);
    
    // Tape entries
    u32 tapePosition = 64 + maxFiles * 32; // data of item 0 starts here
    memset(ptr, 0, 32 * maxFiles);
    for (unsigned n = 0; n < maxFiles; n++) {
        
        if (n >= currentFiles) {
            // Empty tape slot
            ptr += 32;
            continue;
        }
        
        otherArchive->selectItem(n);
        
        // Entry used (1 byte)
        *ptr++ = 0x01;
        
        // File type (1 byte)
        *ptr++ = 0x82;
        
        // Start address (2 bytes)
        u16 startAddr = otherArchive->getDestinationAddrOfItem();
        *ptr++ = LO_BYTE(startAddr);
        *ptr++ = HI_BYTE(startAddr);
        
        // Start address (2 bytes)
        u16 endAddr = startAddr + otherArchive->getSizeOfItem();
        *ptr++ = LO_BYTE(endAddr);
        *ptr++ = HI_BYTE(endAddr);
        
        // Reserved (2 bytes)
        ptr += 2;
        
        // Tape position (4 bytes)
        *ptr++ = LO_BYTE(tapePosition);
        *ptr++ = LO_BYTE(tapePosition >> 8);
        *ptr++ = LO_BYTE(tapePosition >> 16);
        *ptr++ = LO_BYTE(tapePosition >> 24);
        tapePosition += otherArchive->getSizeOfItem();
        
        // Reserved (4 bytes)
        ptr += 4;
        
        // File name (16 bytes)
        strncpy((char *)ptr, (char *)otherArchive->getNameOfItem(), 16);
        for (unsigned i = 0; i < 16; i++, ptr++)
            *ptr = ascii2pet(*ptr);
    }
    
    // File data
    for (unsigned n = 0; n < currentFiles; n++) {
        
        int byte;
        otherArchive->selectItem(n);
        while ((byte = otherArchive->readItem()) != EOF) {
            *ptr++ = (u8)byte;
        }
        
    }
    
    otherArchive->dumpDirectory();
    archive->dumpDirectory();
    archive->debug(FILE_DEBUG, "T64 archive created with other archive of type %s.\n",
                   otherArchive->typeAsString());
    
    return archive;
}

const char *
T64File::getName()
{
	int i,j;
	int first = 0x28;
	int last  = 0x40;
	
	for (j = 0, i = first; i < last; i++, j++) {
		name[j] = (data[i] == 0x20 ? ' ' : data[i]);
		if (j == 255) break; 
	}
	name[j] = 0x00;
	return name;
}

bool
T64File::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AnyC64File::readFromBuffer(buffer, length))
        return false;
    
    // Some T64 archives contain incosistencies. We fix them asap
    (void)repair();
    
    return true;
}

int
T64File::numberOfItems()
{
    return LO_HI(data[0x24], data[0x25]);
}

void
T64File::selectItem(unsigned item)
{
    // Invalidate the file pointer if a non-existing item is requested.
    if (item >= numberOfItems()) {
        iFp = -1;
        return;
    }
    
    // Remember the selection
    selectedItem = item;
    
    // Set file pointer and end of file index
    unsigned i = 0x48 + (item * 0x20);
    iFp = LO_LO_HI_HI(data[i], data[i+1], data[i+2], data[i+3]);
    iEof = iFp + getSizeOfItem();
    
    // Check for inconsistent values. As all inconsistencies should have
    // been ruled out by repair(), the if condition should never hit.
    if (iFp > size || iEof > size) {
        assert(false);
    }
}

const char *
T64File::getTypeOfItemAsString()
{
    assert(selectedItem != -1);
    
    long i = 0x41 + (selectedItem * 0x20);
    if (data[i] != 00)
        return "PRG";
    if (data[i] == 00 && data[i-1] > 0x00)
        return "FRZ";
    return "???";
}

const char *
T64File::getNameOfItem()
{
    assert(selectedItem != -1);
    
    long i,j;
    long first = 0x50 + (selectedItem * 0x20);
    long last  = 0x60 + (selectedItem * 0x20);
    
    if (size < last) {
        name[0] = 0;
    } else {
        for (j = 0, i = first; i < last; i++, j++) {
            name[j] = (data[i] == 0x20 ? ' ' : data[i]);
            if (j == 255) break;
        }
        name[j] = 0x00;
    }
    return name;
}

size_t
T64File::getSizeOfItem()
{
    if (selectedItem < 0)
        return 0;
    
    // Compute start and end of the selected item in memory
    size_t offset = 0x42 + (selectedItem * 0x20);
    u16 startAddrInMemory = LO_HI(data[offset], data[offset + 1]);
    u16 endAddrInMemory = LO_HI(data[offset + 2], data[offset + 3]);
    
    return endAddrInMemory - startAddrInMemory;
}

void
T64File::seekItem(long offset)
{
    size_t i = 0x48 + (selectedItem * 0x20);

    // Move file pointer the the start of the selected item + offset
    iFp = LO_LO_HI_HI(data[i], data[i+1], data[i+2], data[i+3]) + offset;

    // Invalidate file pointer if it is out of range
    if (iFp > size)
        iFp = -1;
}






u16
T64File::getDestinationAddrOfItem()
{
    long i = 0x42 + (selectedItem * 0x20);
    u16 result = LO_HI(data[i], data[i+1]);
    return result;
}



bool
T64File::directoryItemIsPresent(int item)
{
    int first = 0x40 + (item * 0x20);
    int last  = 0x60 + (item * 0x20);
    int i;
    
    // check for zeros...
    if (last < size)
        for (i = first; i < last; i++)
            if (data[i] != 0)
                return true;
    
    return false;
}

bool
T64File::repair()
{
    unsigned i, n;
    u16 noOfItems = numberOfItems();

    //
    // 1. Repair number of items, if this value is zero
    //
    
    if (noOfItems == 0) {

        while (directoryItemIsPresent(noOfItems))
            noOfItems++;

        u16 noOfItemsStatedInHeader = numberOfItems();
        if (noOfItems != noOfItemsStatedInHeader) {
        
            debug(FILE_DEBUG, "Repairing corrupted T64 archive: Changing number of items from %d to %d.\n", noOfItemsStatedInHeader, noOfItems);
        
            data[0x24] = LO_BYTE(noOfItems);
            data[0x25] = HI_BYTE(noOfItems);
            
        }
        assert(noOfItems == numberOfItems());
    }
    
    for (i = 0; i < noOfItems; i++) {

        //
        // 2. Check relative offset information for each item
        //

        // Compute start address in file
        n = 0x48 + (i * 0x20);
        u16 startAddrInContainer = LO_LO_HI_HI(data[n], data[n+1], data[n+2], data[n+3]);

        if (startAddrInContainer >= size) {
            warn("T64 archive is corrupt (offset mismatch). Sorry, can't repair.\n");
            return false;
        }
    
        //
        // 3. Check for file end address mismatches (as created by CONVC64)
        //
        
        // Compute start address in memory
        n = 0x42 + (i * 0x20);
        u16 startAddrInMemory = LO_HI(data[n], data[n+1]);
    
        // Compute end address in memory
        n = 0x44 + (i * 0x20);
        u16 endAddrInMemory = LO_HI(data[n], data[n+1]);
    
        if (endAddrInMemory == 0xC3C6) {

            // Let's assume that the rest of the file data belongs to this file ...
            u16 fixedEndAddrInMemory = startAddrInMemory + (size - startAddrInContainer);

            debug(FILE_DEBUG, "Repairing corrupted T64 archive: Changing end address of item %d from %04X to %04X.\n", i, endAddrInMemory, fixedEndAddrInMemory);

            data[n] = LO_BYTE(fixedEndAddrInMemory);
            data[n+1] = HI_BYTE(fixedEndAddrInMemory);
        }
    }
    
    return 1; // Archive repaired successfully
}
