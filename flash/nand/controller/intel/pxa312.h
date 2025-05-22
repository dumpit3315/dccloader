#pragma once
#include "dcc/bitutils.h"

#define PXA3_NAND_BASE_REG 0x43100000

#define PXA3_REG_NDCR PXA3_NAND_BASE_REG + 0x00
#define PXA3_REG_NDTR0CS0 PXA3_NAND_BASE_REG + 0x04
#define PXA3_REG_NDTR1CS0 PXA3_NAND_BASE_REG + 0x0C
#define PXA3_REG_NDSR PXA3_NAND_BASE_REG + 0x14
#define PXA3_REG_NDPCR PXA3_NAND_BASE_REG + 0x18
#define PXA3_REG_NDBDR0 PXA3_NAND_BASE_REG + 0x1C
#define PXA3_REG_NDBDR1 PXA3_NAND_BASE_REG + 0x20
#define PXA3_REG_NDECCCTRL PXA3_NAND_BASE_REG + 0x28
#define PXA3_REG_NDDB PXA3_NAND_BASE_REG + 0x40
#define PXA3_REG_NDCB0 PXA3_NAND_BASE_REG + 0x48
#define PXA3_REG_NDCB1 PXA3_NAND_BASE_REG + 0x4C
#define PXA3_REG_NDCB2 PXA3_NAND_BASE_REG + 0x50

bitmask PXA3_NDCR_SPARE_EN = {31, 0x1};
bitmask PXA3_NDCR_ECC_EN = {30, 0x1};
bitmask PXA3_NDCR_DMA_EN = {29, 0x1};
bitmask PXA3_NDCR_ND_RUN = {28, 0x1};
bitmask PXA3_NDCR_DWIDTH_C = {27, 0x1};
bitmask PXA3_NDCR_DWIDTH_M = {26, 0x1};
bitmask PXA3_NDCR_PAGE_SZ = {24, 0x1};
bitmask PXA3_NDCR_NCSX = {23, 0x1};
bitmask PXA3_NDCR_CLR_PG_CNT = {20, 0x1};
bitmask PXA3_NDCR_STOP_ON_UNCOR = {19, 0x1};
bitmask PXA3_NDCR_RD_ID_CNT = {16, 0x7};
bitmask PXA3_NDCR_RA_START = {15, 0x1};
bitmask PXA3_NDCR_PG_PER_BLK = {14, 0x1};
bitmask PXA3_NDCR_ND_ARB_EN = {12, 0x1};
bitmask PXA3_NDCR_INTERRUPT = {0, 0xfff};

bitmask PXA3_NDSR_MASK = {0, 0xfff};
bitmask PXA3_NDSR_RDY = {12, 0x1};
bitmask PXA3_NDSR_FLASH_RDY = {11, 0x1};
bitmask PXA3_NDSR_CS0_PAGED = {10, 0x1};
bitmask PXA3_NDSR_CS1_PAGED = {9, 0x1};
bitmask PXA3_NDSR_CS0_CMDD = {8, 0x1};
bitmask PXA3_NDSR_CS1_CMDD = {7, 0x1};
bitmask PXA3_NDSR_CS0_BBD = {6, 0x1};
bitmask PXA3_NDSR_CS1_BBD = {5, 0x1};
bitmask PXA3_NDSR_UNCORERR = {4, 0x1};
bitmask PXA3_NDSR_CORERR = {3, 0x1};
bitmask PXA3_NDSR_WRDREQ = {2, 0x1};
bitmask PXA3_NDSR_RDDREQ = {1, 0x1};
bitmask PXA3_NDSR_WRCMDREQ = {0, 0x1};

bitmask PXA3_NDCB_LEN_OVRD = {28, 0x1};
bitmask PXA3_NDCB_ST_ROW_EN = {26, 0x1};
bitmask PXA3_NDCB_AUTO_RS = {25, 0x1};
bitmask PXA3_NDCB_CSEL = {24, 0x1};
bitmask PXA3_NDCB_EXT_CMD_TYPE = {29, 0x7};
bitmask PXA3_NDCB_CMD_TYPE = {21, 0x7};
bitmask PXA3_NDCB_NC = {20, 0x1};
bitmask PXA3_NDCB_DBC = {19, 0x1};
bitmask PXA3_NDCB_ADDR_CYC = {16, 0x7};
bitmask PXA3_NDCB_CMD2 = {8, 0xff};
bitmask PXA3_NDCB_CMD1 = {0, 0xff};

typedef enum {
    PXA3_NDCB_TYPE_READ,
    PXA3_NDCB_TYPE_WRITE,
    PXA3_NDCB_TYPE_ERASE,
    PXA3_NDCB_TYPE_READID,
    PXA3_NDCB_TYPE_STATUS_READ,
    PXA3_NDCB_TYPE_RESET
} PXA3_NDCB_TYPE;