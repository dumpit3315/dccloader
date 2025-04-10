#include "dcc/dn_dcc_proto.h"
#include "flash/cfi/cfi.h"

#define DCC_BUFFER_SIZE 0x20000
static uint8_t compBuf[DCC_BUFFER_SIZE + 0x1000];

// dcc code
void dcc_main(uint32_t BaseAddress1, uint32_t BaseAddress2, uint32_t BaseAddress3, uint32_t BaseAddress4) {
    DCCMemory mem = { 0 };
    uint32_t BUF_INIT[32];
    uint32_t dcc_init_size = 0;
    uint32_t dcc_comp_packet_size;
    // uint32_t dcc_offset;

    CFI_Probe(BaseAddress1, &mem);
    switch (mem.type) {
        case MEMTYPE_NOR:
            BUF_INIT[0] = DCC_MEM_OK | (DCC_MEM_NOR << 16);
            BUF_INIT[1] = mem.manufacturer | (mem.device_id << 16);
            BUF_INIT[2] = DCC_MEM_NOR_INFO(mem.bit_width, mem.size >> 20);
            BUF_INIT[3] = DCC_MEM_OK | (0x5 << 16);
            BUF_INIT[4] = DCC_BUFFER_SIZE;
            dcc_init_size = 5;
            break;

        case MEMTYPE_NAND:
            BUF_INIT[0] = DCC_MEM_OK | (mem.page_size << 16);
            BUF_INIT[1] = mem.manufacturer | (mem.device_id << 16);
            BUF_INIT[2] = DCC_MEM_OK | (0x5 << 16);
            BUF_INIT[3] = DCC_BUFFER_SIZE;
            dcc_init_size = 4;
            break;

        default:
            BUF_INIT[0] = DCC_MEM_OK | (DCC_MEM_NONE << 16);
            BUF_INIT[1] = mem.manufacturer | (mem.device_id << 16);
            BUF_INIT[2] = DCC_MEM_OK | (0x5 << 16);
            BUF_INIT[3] = DCC_BUFFER_SIZE;
            dcc_init_size = 4;

    }

    DN_Packet_Send((uint8_t *)BUF_INIT, 4 * dcc_init_size);

    uint32_t flashIndex;
    uint32_t srcOffset;
    uint32_t srcSize;

    while (1) {
        wdog_reset();
        uint32_t cmd = DN_Packet_DCC_Read();

        switch (cmd & 0xff) {
            case CMD_CONFIGURE:
                for (int c = 0; c < (cmd >> 0x10); c += 4) {
                    DN_Packet_DCC_Read();
                }
                DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(0x38, 0x6));
                break;

            case CMD_GETINFO:
                DN_Packet_Send((uint8_t *)BUF_INIT, 4 * dcc_init_size);
                break;
                
            case CMD_GETMEMSIZE:
                flashIndex = (cmd >> 8) & 0xff;
                if (flashIndex == 0) {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(0x21, 0));
                } else if (flashIndex == 1) {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(0x21, mem.size >> 20));
                } else {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_FLASH_NOENT, flashIndex));
                }
                break;

            case CMD_READ:
                srcOffset = DN_Packet_DCC_Read();
                srcSize = DN_Packet_DCC_Read();
                flashIndex = (cmd >> 8) & 0xff;
                uint8_t algo = (cmd >> 24) & 0xff;

                if (srcSize > DCC_BUFFER_SIZE) {
                    DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_INVALID_ARGS));
                    continue;
                }

                if (flashIndex == 0) {
                Jump_Read_NOR:
                    switch (algo) {
                        case CMD_READ_COMP_NONE:
                            dcc_comp_packet_size = DN_Packet_CompressNone((uint8_t *)srcOffset, srcSize, compBuf);
                            break;

                        case CMD_READ_COMP_RLE:
                            dcc_comp_packet_size = DN_Packet_Compress((uint8_t *)srcOffset, srcSize, compBuf);
                            break;

                        case CMD_READ_COMP_LZO:
                            dcc_comp_packet_size = DN_Packet_Compress2((uint8_t *)srcOffset, srcSize, compBuf);
                            break;

                        case CMD_READ_COMP_LZ4:
                            dcc_comp_packet_size = DN_Packet_Compress3((uint8_t *)srcOffset, srcSize, compBuf);
                            break;

                        default:
                            DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_INVALID_ARGS));
                            continue;
                    }
                    
                    DN_Packet_Send(compBuf, dcc_comp_packet_size);
                } else if (flashIndex == 1) {
                    switch (mem.type) {
                        case MEMTYPE_NAND:
                            break; // Implement NAND page routines

                        case MEMTYPE_NOR:
                        default:
                            goto Jump_Read_NOR;
                    }
                } else {
                    DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_FLASH_NOENT));
                }

                break;

            case CMD_ERASE:
                srcOffset = DN_Packet_DCC_Read();
                srcSize = DN_Packet_DCC_Read();
                flashIndex = (cmd >> 8) & 0xff;

                DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_WPROT_ERROR, flashIndex)); // TODO
                break;

            case CMD_WRITE:
                // uint32_t pAddrStart = DN_Packet_DCC_Read();
                // uint32_t dataPackN = DN_Packet_DCC_Read();
                // uint8_t progType = (cmd >> 8) & 0xff;
                // uint8_t flashIndex = (cmd >> 16) & 0xff;

                // DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_WPROT_ERROR, flashIndex)); // TODO
                // break;

            default:
                DN_Packet_Send_One(DCC_BAD_COMMAND(cmd));
        }
    }
}