#pragma once
#include "dcc/dn_dcc_proto.h"

typedef struct {
	uint32_t dev_id;
	uint32_t page_size;
	uint32_t chip_size;
	uint32_t block_size;
	uint32_t bits;
} nand_info;

extern nand_info flash_ids[];

typedef enum {
	NAND_STATUS_FAIL = 0x01,
	NAND_STATUS_FAIL_N1 = 0x02,
	NAND_STATUS_TRUE_READY = 0x20,
	NAND_STATUS_READY = 0x40,
	NAND_STATUS_WP = 0x80,
} NANDStatus;

typedef enum {
	/* Standard NAND flash commands */
	NAND_CMD_READ0 = 0x0,
	NAND_CMD_READ1 = 0x1,
	NAND_CMD_RNDOUT = 0x5,
	NAND_CMD_PAGEPROG = 0x10,
	NAND_CMD_READOOB = 0x50,
	NAND_CMD_ERASE1 = 0x60,
	NAND_CMD_STATUS = 0x70,
	NAND_CMD_STATUS_MULTI = 0x71,
	NAND_CMD_SEQIN = 0x80,
	NAND_CMD_RNDIN = 0x85,
	NAND_CMD_READID = 0x90,
	NAND_CMD_ERASE2 = 0xd0,
	NAND_CMD_RESET = 0xff,

	/* Extended commands for large page devices */
	NAND_CMD_READSTART = 0x30,
	NAND_CMD_RNDOUTSTART = 0xE0,
	NAND_CMD_CACHEDPROG = 0x15,
} NANDCommands;

DCC_RETURN NAND_Ctrl_Probe(DCCMemory *mem);
DCC_RETURN NAND_Ctrl_Read(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page);