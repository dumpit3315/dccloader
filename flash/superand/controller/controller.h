#pragma once
#include "dcc/dn_dcc_proto.h"

typedef struct {
	uint32_t dev_id;
	uint32_t page_size;
	uint32_t chip_size;
	uint32_t block_size;
	uint32_t bits;
} superand_info;

static superand_info flash_ids[] = {
    /* 16MB */
    {0x51, 0x200, 0x01000000, 0x4000, 8},
    {0x52, 0x200, 0x01000000, 0x4000, 8},
    {0x53, 0x200, 0x01000000, 0x4000, 16},
    {0x54, 0x200, 0x01000000, 0x4000, 16},

    /* 32MB */
    // {0x48, 0x200, 0x02000000, 0x4000, 8},
    {0x49, 0x200, 0x02000000, 0x4000, 8},
    {0x4a, 0x200, 0x02000000, 0x4000, 16},
    // {0x4b, 0x200, 0x02000000, 0x4000, 16},

    /* 64MB */
    {0x58, 0x200, 0x04000000, 0x4000, 8},
    // {0x59, 0x200, 0x04000000, 0x4000, 8},
    // {0x5a, 0x200, 0x04000000, 0x4000, 16},
    {0x5b, 0x200, 0x04000000, 0x4000, 16},
};

typedef enum {
	SUPERAND_CMD_READ = 0x0,
	SUPERAND_CMD_READ_SEQ = 0xF,
	SUPERAND_CMD_PAGEPROG = 0x10,
    SUPERAND_CMD_EXTMODE = 0x11,
    SUPERAND_CMD_REWRITE = 0x1F,
    SUPERAND_CMD_ERASE1 = 0x60,
	SUPERAND_CMD_STATUS = 0x70,
    SUPERAND_CMD_SEQIN = 0x80,
	SUPERAND_CMD_READID = 0x90,
    SUPERAND_CMD_STANDBY_SET = 0xC0,
    SUPERAND_CMD_STANDBY_REL = 0xC1,
    SUPERAND_CMD_ERASE2 = 0xD0,
    SUPERAND_CMD_READ_EXIT = 0xF0
} SuperANDCommands;

DCC_RETURN SuperAND_Ctrl_Probe(DCCMemory *mem);
DCC_RETURN SuperAND_Ctrl_Read(DCCMemory *mem, uint8_t *page_buf, uint32_t page);