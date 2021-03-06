// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _BASIC_INC
#define _BASIC_INC

// General Includes
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <time.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sched.h>
#include <assert.h>
#include <math.h>
#include <ctype.h> 

#include "C64Config.h"
#include "C64Types.h"
#include "C64Constants.h"

//! @brief    Two bit binary value
typedef u8 uint2_t;

//! @brief    Integrity check
inline bool is_uint2_t(uint2_t value) { return value < 4; }

//! @brief    Three bit binary value
typedef u8 uint3_t;

//! @brief    Integrity check
inline bool is_uint3_t(uint2_t value) { return value < 8; }

//! @brief    Four bit binary value
typedef u8 uint4_t;

//! @brief    Integrity check
inline bool is_uint4_t(uint4_t value) { return value < 16; }

//! @brief    Five bit binary value
typedef u8 uint5_t;

//! @brief    Integrity check
inline bool is_uint5_t(uint5_t value) { return value < 32; }


//
//! @functiongroup Handling low level data objects
//

//! @brief    Returns the high byte of a u16 value.
#define HI_BYTE(x) (u8)((x) >> 8)

//! @brief    Returns the low byte of a u16 value.
#define LO_BYTE(x) (u8)((x) & 0xFF)

//! @brief    Specifies a larger integer in little endian byte format
#define LO_HI(x,y) (u16)((y) << 8 | (x))
#define LO_LO_HI(x,y,z) (u32)((z) << 16 | (y) << 8 | (x))
#define LO_LO_HI_HI(x,y,z,w) (u32)((w) << 24 | (z) << 16 | (y) << 8 | (x))

//! @brief    Specifies a larger integer in big endian byte format
#define HI_LO(x,y) (u16)((x) << 8 | (y))
#define HI_HI_LO(x,y,z) (u32)((x) << 16 | (y) << 8 | (z))
#define HI_HI_LO_LO(x,y,z,w) (u32)((x) << 24 | (y) << 16 | (z) << 8 | (w))

//! @brief    Returns a certain byte of a larger integer
#define BYTE0(x) LO_BYTE(x)
#define BYTE1(x) LO_BYTE((x) >> 8)
#define BYTE2(x) LO_BYTE((x) >> 16)
#define BYTE3(x) LO_BYTE((x) >> 24)

//! @brief    Returns a non-zero value if the n-th bit is set in x.
#define GET_BIT(x,nr) ((x) & (1 << (nr)))

//! @brief    Sets a single bit.
#define SET_BIT(x,nr) ((x) |= (1 << (nr)))

//! @brief    Clears a single bit.
#define CLR_BIT(x,nr) ((x) &= ~(1 << (nr)))

//! @brief    Toggles a single bit.
#define TOGGLE_BIT(x,nr) ((x) ^= (1 << (nr)))

//! @brief    Sets a single bit to 0 (value == 0) or 1 (value != 0)
#define WRITE_BIT(x,nr,value) ((value) ? SET_BIT(x, nr) : CLR_BIT(x, nr))

//! @brief    Copies a single bit from x to y.
#define COPY_BIT(x,y,nr) ((y) = ((y) & ~(1 << (nr)) | ((x) & (1 << (nr)))))

//! @brief    Returns true if value is rising when switching from x to y
#define RISING_EDGE(x,y) (!(x) && (y))

//! @brief    Returns true if bit n is rising when switching from x to y
#define RISING_EDGE_BIT(x,y,n) (!((x) & (1 << (n))) && ((y) & (1 << (n))))

//! @brief    Returns true if value is falling when switching from x to y
#define FALLING_EDGE(x,y) ((x) && !(y))

//! @brief    Returns true if bit n is falling when switching from x to y
#define FALLING_EDGE_BIT(x,y,n) (((x) & (1 << (n))) && !((y) & (1 << (n))))


//
//! @functiongroup Handling buffers
//

//! @brief    Writes a byte value into a buffer.
inline void write8(u8 **ptr, u8 value) { *((*ptr)++) = value; }

//! @brief    Writes a word value into a buffer in big endian format.
inline void write16(u8 **ptr, u16 value) {
    write8(ptr, (u8)(value >> 8)); write8(ptr, (u8)value); }

//! @brief    Writes a double byte value into a buffer in big endian format.
inline void write32(u8 **ptr, u32 value) {
    write16(ptr, (u16)(value >> 16)); write16(ptr, (u16)value); }

//! @brief    Writes a quad word value into a buffer in big endian format.
inline void write64(u8 **ptr, u64 value) {
    write32(ptr, (u32)(value >> 32)); write32(ptr, (u32)value); }

//! @brief    Writes a memory block into a buffer in big endian format.
inline void writeBlock(u8 **ptr, u8 *values, size_t length) {
    memcpy(*ptr, values, length); *ptr += length; }

//! @brief    Writes a word memory block into a buffer in big endian format.
inline void writeBlock16(u8 **ptr, u16 *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(u16); i++) write16(ptr, values[i]); }

//! @brief    Writes a double word memory block into a buffer in big endian format.
inline void writeBlock32(u8 **ptr, u32 *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(u32); i++) write32(ptr, values[i]); }

//! @brief    Writes a quad word memory block into a buffer in big endian format.
inline void writeBlock64(u8 **ptr, u64 *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(u64); i++) write64(ptr, values[i]); }


//! @brief    Reads a byte value from a buffer.
inline u8 read8(u8 **ptr) { return (u8)(*((*ptr)++)); }

//! @brief    Reads a word value from a buffer in big endian format.
inline u16 read16(u8 **ptr) {
    return ((u16)read8(ptr) << 8) | (u16)read8(ptr); }

//! @brief    Reads a double word value from a buffer in big endian format.
inline u32 read32(u8 **ptr) {
    return ((u32)read16(ptr) << 16) | (u32)read16(ptr); }

//! @brief    Reads a quad word value from a buffer in big endian format.
inline u64 read64(u8 **ptr) {
    return ((u64)read32(ptr) << 32) | (u64)read32(ptr); }

//! @brief    Reads a memory block from a buffer.
inline void readBlock(u8 **ptr, u8 *values, size_t length) {
    memcpy(values, *ptr, length); *ptr += length; }

//! @brief    Reads a word block from a buffer in big endian format.
inline void readBlock16(u8 **ptr, u16 *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(u16); i++) values[i] = read16(ptr); }

//! @brief    Reads a double word block from a buffer in big endian format.
inline void readBlock32(u8 **ptr, u32 *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(u32); i++) values[i] = read32(ptr); }

//! @brief    Reads a quad word block from a buffer in big endian format.
inline void readBlock64(u8 **ptr, u64 *values, size_t length) {
    for (unsigned i = 0; i < length / sizeof(u64); i++) values[i] = read64(ptr); }


//
//! @functiongroup Converting low level data objects
//

/*! @brief    Translates a PETSCII string to a unichar array.
 *  @details  This functions creates unicode characters compatible with the
 *            C64ProMono font. The target font supports four different mapping
 *            tables starting at different base addresses:
 *
 *            0xE000 : Unshifted (only upper case characters)  
 *            0xE100 : Shifted   (upper and lower case characters)
 *            0xE200 : Unshifted, reversed 
 *            0xE300 : Shifted, reversed
 *
 *  @note     A maximum of max characters are translated. 
 *            The unicode array will always be terminated by a NULL character.
 */
void translateToUnicode(const char *petscii, u16 *unichars,
                        u16 base, size_t max);

//! @brief    Returns the number of characters in a null terminated unichar array
size_t strlen16(const u16 *unichars);

/*! @brief    Converts a PETSCII character to a printable character.
 *  @details  Replaces all unprintable characters by subst.
 */
u8 petscii2printable(u8 c, u8 subst);

/*! @brief    Converts an ASCII character to a PETSCII character.
 *  @details  This function translates into the unshifted PET character set.
 *            I.e., lower case characters are converted to uppercase characters.
 *  @result   Returns ' ' for ASCII characters with no PETSCII representation.
 */
u8 ascii2pet(u8 asciichar);

//! @brief    Converts an ASCII string into a PETSCII string.
/*! @details  Applies function ascii2pet to all characters of a string.
 */
void ascii2petStr(char *str);

//! @brief    Writes an u8 value into a string in decimal format
void sprint8d(char *s, u8 value);

//! @brief    Writes an u8 value into a string in hexadecimal format
void sprint8x(char *s, u8 value);

//! @brief    Writes an u8 value into a string in binary format
void sprint8b(char *s, u8 value);

//! @brief    Writes an u16 value into a string in decimal format
void sprint16d(char *s, u16 value);

//! @brief    Writes an u16 value into a string in hexadecimal format
void sprint16x(char *s, u16 value);

//! @brief    Writes an u16 value into a string in binary format
void sprint16b(char *s, u16 value);


//
//! @functiongroup Handling file and path names
//

/*! @brief    Extracts filename from a path
 *  @details  Returns a newly created string. You need to delete it manually.
 */
char *extractFilename(const char *path);

/*! @brief    Extracts file suffix from a path
 *  @details  Returns a newly created string. You need to delete it manually.
 */
char *extractSuffix(const char *path);

/*! @brief    Extracts filename from a path without its suffix
 *  @details  Returns a newly created string. You need to delete it manually.
 */
char *extractFilenameWithoutSuffix(const char *path);

/*! @brief    Check file suffix
 *  @details  The function is used for determining the type of a file. 
 */
bool checkFileSuffix(const char *filename, const char *suffix);

//! @brief    Returns the size of a file in bytes
long getSizeOfFile(const char *filename);

/*! @brief    Checks the size of a file
 *  @details  The function is used for validating the size of a file.
 *  @param    filename Path and name of the file to investigate
 *  @param    min Expected minimum size (-1 if no lower bound exists)
 *  @param    max Expected maximum size (-1 if no upper bound exists)
 */
bool checkFileSize(const char *filename, long min, long max);

/*! @brief    Checks the header signature (magic bytes) of a file.
 *  @details  This function is used for determining the type of a file.
 *  @param    File name, must not be Null
 *  @param    Expected byte sequence
 *  @param    Length of the expected byte sequence in bytes
 */
bool matchingFileHeader(const char *path, const u8 *header, size_t length);

/*! @brief    Checks the header signature (magic bytes) of a buffer.
 *  @details  This function is used for determining the type of a file.
 *  @param    Pointer to buffer, must not be NULL
 *  @param    Expected byte sequence
 *  @param    Length of the expected byte sequence in bytes
 */
bool matchingBufferHeader(const u8 *buffer, const u8 *header, size_t length);

/*! @brief    Checks the magic bytes of a file.
 *  @details  The function is used for determining the type of a file.
 *  @param    filename  Path and name of the file to investigate.
 *  @param    header    Expected byte sequence, terminated by 0x00.
 *  @return   Returns   true iff magic bytes match.
 *  @deprecated Use matchingFileHeader() instead.
*/
// bool checkFileHeader(const char *filename, const u8 *header);


//
//! @functiongroup Managing time
//

/*! @brief    Application launch time in seconds
 *  @details  The value is read by function msec for computing the elapsed
 *            number of microseconds. 
 */
// extern long tv_base;

//! @brief    Return the number of elapsed microseconds since program launch.
// u64 usec();

//! @brief    Reads the real-time clock (1/10th seconds).
u8 localTimeSecFrac();

//! @brief    Reads the real-time clock (seconds).
u8 localTimeSec();

//! @brief    Reads the real-time clock (minutes).
u8 localTimeMin();

//! @brief    Reads the real-time clock (hours).
u8 localTimeHour();

//! @brief    Put the current thread to sleep for a certain amount of time.
void sleepMicrosec(unsigned usec);

/*! @brief    Sleeps until kernel timer reaches kernelTargetTime
 *  @param    kernelEarlyWakeup: To increase timing precision, the function
 *            wakes up the thread earlier by this amount and waits actively in
 *            a delay loop until the deadline is reached.
 *  @return   Overshoot time (jitter), measured in kernel time. Smaller values
 *            are better, 0 is best.
 */
i64 sleepUntil(u64 kernelTargetTime, u64 kernelEarlyWakeup);


//
//! @functiongroup Computing checksums
//

//! @brief    Returns the FNV-1a seed value.
inline u32 fnv_1a_init32() { return 0x811c9dc5; }
inline u64 fnv_1a_init64() { return 0xcbf29ce484222325; }

//! @brief    Performs a single iteration of the FNV-1a hash algorithm.
inline u32 fnv_1a_it32(u32 prev, u32 value) { return (prev ^ value) * 0x1000193; }
inline u64 fnv_1a_it64(u64 prev, u64 value) { return (prev ^ value) * 0x100000001b3; }

//! @brief    Computes a FNV-1a for a given buffer
u32 fnv_1a_32(u8 *addr, size_t size);
u64 fnv_1a_64(u8 *addr, size_t size);

#endif
