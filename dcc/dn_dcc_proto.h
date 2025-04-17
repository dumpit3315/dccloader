#pragma once
#include "plat.h"

typedef uint32_t DCC_RETURN;

typedef enum {
    MEMTYPE_NONE,
    MEMTYPE_NAND,
    MEMTYPE_NOR,
    MEMTYPE_ONENAND,
    MEMTYPE_SUPERAND,
    MEMTYPE_AND,
    MEMTYPE_AG_AND,
} MemTypes;

typedef struct {
    uint8_t manufacturer;
    uint16_t device_id;
    uint8_t bit_width;
    uint32_t page_size;
    uint32_t block_size;
    uint32_t size;
    uint32_t nor_cmd_set;
    uint32_t base_offset;
    MemTypes type;
} DCCMemory;

typedef struct {
    DCC_RETURN (*initialize)(DCCMemory *mem, uint32_t offset);
    DCC_RETURN (*read)(DCCMemory *mem, uint32_t offset, uint32_t size, uint8_t *dest, uint32_t *dest_size);
} Driver;

typedef struct {
    Driver *driver;
    uint32_t base_offset;
} Device;

// RIFF DCC Commands
// Read Command: 52 01 00 00 00 00 8E 05 00 00 02 00
// Returns: 01 00 00 00 (Len) (Data) if success, ff (Status code) 00 00 otherwise
#define CMD_READ 0x52 // Read, READ_MEMORY command structure
#define CMD_READ_COMP_RLE 0x0 // RLE
#define CMD_READ_COMP_NONE 0x40 // Uncompressed (non-standard)
#define CMD_READ_COMP_LZO 0x80 // LZO (non-standard)
#define CMD_READ_COMP_LZ4 0xc0 // LZ4 (non-standard)
#define CMD_READ_RESP_FAIL(x) (0xff | (x << 8)) // Read error response code

// Write Command: 46 00 01 00 00 00 8E 05 01 00 00 00 04 00 00 00 DD DD DD DD CC CC CC CC
// Returns: Status code followed with target id
#define CMD_WRITE 0x46 // Write, WRITE_MEMORY command structure
#define CMD_WRITE_COMP_NONE 0x0 // Uncompressed
#define CMD_WRITE_COMP_RLE 0x1 // RLE
#define CMD_WRITE_COMP_LZO 0x2 // LZO (non-standard)
#define CMD_WRITE_COMP_LZ4 0x3 // LZ4 (non-standard)

// Erase Command: 45 01 00 00 00 00 8E 05 00 00 02 00
// Returns: Status code followed with target id
#define CMD_ERASE 0x45 // Erase, READ_MEMORY command structure

// Configure: 43 00 08 00
// Returns: 0x638 status code
#define CMD_CONFIGURE 0x43

// Get Info command: 49 00 00 00
// Returns: Initialization data
#define CMD_GETINFO 0x49

// Get Memory Size command: 4d 00 00 00
// Returns: Status code followed with memory size in MB
#define CMD_GETMEMSIZE 0x4d

// Status code for Write/Erase
#define CMD_WRITE_ERASE_STATUS(code, target) (code | (target << 8))

// RIFF DCC Probe Responses
#define DCC_MEM_OK 0x4B4F // OK result, followed with flash ID and type
// Memory types
#define DCC_MEM_NONE   0xffff // Probe failure, followed with error code in Word 1
// NAND
#define DCC_MEM_NAND_U 0x0 // Uninitialized
#define DCC_MEM_NAND_S 0x200 // Small page NAND (B: 0x4000)
#define DCC_MEM_NAND_L 0x800 // Large page NAND (B: 0x20000)
#define DCC_MEM_NAND_X 0x1000 // Extra-large page NAND (B: 0x40000)
// Extended memory info
// for NOR, RIFF computes size by summing all erase block regions.
// (y + 1) * (z << 8), y representing the first uint16 value, and z representing the second uint16 value
// also used for OneNAND, and other NAND-like devices
#define DCC_MEM_EXTENDED_FLAG(no_spare) (0x3 | (no_spare << 3))
#define DCC_MEM_EXTENDED(no_spare, page_size, block_size, size_mb) (DCC_MEM_EXTENDED_FLAG(no_spare) | (DN_Log2(page_size) << 8) | (DN_Log2(block_size) << 16) | (DN_Log2(size_mb) << 24))
// used to set buffer size
#define DCC_MEM_BUFFER(separate) (0x5 | (separate << 8))

// Response codes (For troubleshooting, refer to USB capture between RIFF and Loader)
#define DCC_BAD_COMMAND(c) ((c << 8) | 0x20) // Unknown command, command code follows after an error code
#define DCC_OK             0x21 // All OK
#define DCC_CHECKSUM_ERROR 0x22 // Checksum failure
#define DCC_INVALID_ARGS   0x23 // Invalid arguments
#define DCC_ERASE_ERROR    0x24 // Erase error
#define DCC_PROGRAM_ERROR  0x25 // Write error
#define DCC_PROBE_ERROR    0x26 // Device probe failed
#define DCC_ASSERT_ERROR   0x27 // Ready flag timeout
#define DCC_READ_ERROR     0x28 // Read error
#define DCC_W_ASSERT_ERROR 0x2A // Ready flag timeout during write
#define DCC_E_ASSERT_ERROR 0x2B // Ready flag timeout during erase
#define DCC_ROFS_ERROR     0x2D // Cannot write to read-only memory
#define DCC_E_UNK_ERROR    0x2E // Unknown erase error, Please file a bug report
#define DCC_WUPROT_TIMEOUT 0x2F // Write unprotect timeout
#define DCC_WUPROT_ERROR   0x30 // Write unprotect failed
#define DCC_W_UNK_ERROR    0x31 // Unknown write error, Please file a bug report
#define DCC_FLASH_NOENT    0x37 // Flash with this ID is not probed/not found
#define DCC_WPROT_ERROR    0x3C // Read-only memory or Write/Erase routines not implemented
#define DCC_NOMEM_ERROR    0x3D // Not enough memory

// Functions
uint32_t DN_Packet_Compress(uint8_t *src, uint32_t size, uint8_t *dest);
#if HAVE_MINILZO
uint32_t DN_Packet_Compress2(uint8_t *src, uint32_t size, uint8_t *dest);
#endif
#if HAVE_LZ4
uint32_t DN_Packet_Compress3(uint8_t *src, uint32_t size, uint8_t *dest);
#endif
uint32_t DN_Packet_CompressNone(uint8_t *src, uint32_t size, uint8_t *dest);
uint32_t DN_Calculate_CRC32(uint32_t crc, uint8_t* data, uint32_t len);
uint32_t DN_Packet_DCC_Send(uint32_t data);
uint32_t DN_Packet_DCC_Read(void);
void DN_Packet_Send(uint8_t *src, uint32_t size);
void DN_Packet_Send_One(uint32_t data);
uint32_t DN_Log2(uint32_t value);

// Watchdog
extern void wdog_reset(void);