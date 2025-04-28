// Intel XScale PXA312 NAND Controller
// Found in Samsung SGH-i740 and i780
// SGH-i900 uses PXA3xx series, but uses standard OneNAND flash registers at 0x10000000 (K5W2G1HACA)
#include "../controller.h"
#include "pxa312.h"

#define PXA3_CMD(cmd, cmd_type, addr_c) (cmd | (cmd_type << PXA3_NDCB_CMD_TYPE.bit_pos) | (addr_c << PXA3_NDCB_ADDR_CYC.bit_pos))
#define PXA3_CMD_DBC(cmd1, cmd2, cmd_type, addr_c) (cmd1 | (cmd2 << 8) | (cmd_type << PXA3_NDCB_CMD_TYPE.bit_pos) | (addr_c << PXA3_NDCB_ADDR_CYC.bit_pos) | (1 << PXA3_NDCB_DBC.bit_pos))


void inline PXA3_Start() {
    WRITE_U32(PXA3_REG_NDSR, 0xfff);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_ND_RUN, 1);

    do { wdog_reset(); } while (!(GET_BIT32(PXA3_REG_NDSR, PXA3_NDSR_WRCMDREQ)));
}

void inline PXA3_Stop() {
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_ND_RUN, 0);
}

void inline PXA3_NAND_Reset() {
    WRITE_U32(PXA3_REG_NDCB0, PXA3_CMD(NAND_CMD_RESET, PXA3_NDCB_TYPE_RESET, 0));
    WRITE_U32(PXA3_REG_NDCB1, 0);
    WRITE_U32(PXA3_REG_NDCB2, 0);

    do { wdog_reset(); } while (!(GET_BIT32(PXA3_REG_NDSR, PXA3_NDSR_CS0_CMDD)));
    do { wdog_reset(); } while (!(GET_BIT32(PXA3_REG_NDSR, PXA3_NDSR_RDY)));
} 

uint32_t inline PXA3_NAND_ReadId() {
    WRITE_U32(PXA3_REG_NDCB0, PXA3_CMD(NAND_CMD_READID, PXA3_NDCB_TYPE_READID, 0));
    WRITE_U32(PXA3_REG_NDCB1, 0);
    WRITE_U32(PXA3_REG_NDCB2, 0);

    do { wdog_reset(); } while (!(GET_BIT32(PXA3_REG_NDSR, PXA3_NDSR_RDDREQ)));

    return READ_U32(PXA3_REG_NDDB);
} 

uint32_t inline PXA3_Check_Status() {
    return 1;
}

DCC_RETURN NAND_Ctrl_Probe(DCCMemory *mem) {
    wdog_reset();
    mem->type = MEMTYPE_NONE;

    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_DMA_EN, 0);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_ECC_EN, 0);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_ND_RUN, 0);

    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_ND_ARB_EN, 0);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_SPARE_EN, 1);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_RD_ID_CNT, 4);

    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_RA_START, 0);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_PG_PER_BLK, 0);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_PAGE_SZ, 0);

    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_DWIDTH_C, 0);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_DWIDTH_M, 0);

    PXA3_Start();

    PXA3_NAND_Reset();
    uint32_t nand_idcode = PXA3_NAND_ReadId();

    uint8_t mfr_id = (uint8_t)nand_idcode;
    uint8_t dev_id = (uint8_t)(nand_idcode >> 8);
    uint8_t ext_id = (uint8_t)(nand_idcode >> 24);

    for (int i = 0; flash_ids[i].dev_id; i++) {
        if (dev_id == (uint8_t)flash_ids[i].dev_id) {
            mem->device_id = (uint16_t)dev_id;
            mem->manufacturer = (uint16_t)mfr_id;
            mem->bit_width = flash_ids[i].bits;
            mem->block_size = flash_ids[i].block_size;
            mem->page_size = flash_ids[i].page_size;
            mem->size = flash_ids[i].chip_size;
            mem->type = MEMTYPE_NAND;
            break;
        }
    }

    if (mem->type != MEMTYPE_NAND) return DCC_PROBE_ERROR;

    if (mem->page_size == 0) {
        mem->page_size = 1 << (10 + (ext_id & 3));

        switch ((ext_id >> 4) & 3) {
            case 0:
                mem->block_size = 64 << 10;
                break;
            case 1:
                mem->block_size = 128 << 10;
                break;
            case 2:
                mem->block_size = 256 << 10;
                break;
            case 3:
                mem->block_size = 512 << 10;
                break;
        }

        mem->device_id |= ext_id << 8;
    }
    PXA3_Stop();

    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_RA_START, mem->page_size > 512 ? 1 : 0);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_PG_PER_BLK, mem->page_size > 512 ? 1 : 0);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_PAGE_SZ, mem->page_size > 512 ? 1 : 0);

    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_DWIDTH_C, mem->bit_width == 16 ? 1 : 0);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_DWIDTH_M, mem->bit_width == 16 ? 1 : 0);
    SET_BIT32(PXA3_REG_NDCR, PXA3_NDCR_ECC_EN, 1);

    PXA3_Start();
    PXA3_NAND_Reset();
    PXA3_Stop();

    return DCC_OK;
}

DCC_RETURN NAND_Ctrl_Read(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page) {
    wdog_reset();
    PXA3_Start();

    if (mem->page_size <= 512) {
        WRITE_U32(PXA3_REG_NDCB0, PXA3_CMD(NAND_CMD_READ0, PXA3_NDCB_TYPE_READ, 4));
        WRITE_U32(PXA3_REG_NDCB1, page << 8);
        WRITE_U32(PXA3_REG_NDCB2, page >> 24);
    } else {
        WRITE_U32(PXA3_REG_NDCB0, PXA3_CMD_DBC(NAND_CMD_READ0, NAND_CMD_READSTART, PXA3_NDCB_TYPE_READ, 5));
        WRITE_U32(PXA3_REG_NDCB1, page << 16);
        WRITE_U32(PXA3_REG_NDCB2, page >> 16);
    }
    
    do { wdog_reset(); } while (!(GET_BIT32(PXA3_REG_NDSR, PXA3_NDSR_RDDREQ)));

    for (int i = 0; i < (mem->page_size >> 2); i++) {
        wdog_reset();
        ((uint32_t *)(page_buf))[i] = READ_U32(PXA3_REG_NDDB);
    }

    for (int i = 0; i < ((mem->page_size >> 5) >> 2); i++) {
        wdog_reset();
        ((uint32_t *)(spare_buf))[i] = READ_U32(PXA3_REG_NDDB);
    }
    
    PXA3_Stop();
    return DCC_OK;
}