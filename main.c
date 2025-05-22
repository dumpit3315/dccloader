#include "dcc/dn_dcc_proto.h"
#include "flash/cfi/cfi.h"
#include "devices.h"

typedef DCC_RETURN DCC_INIT_PTR(DCCMemory *mem, uint32_t offset);
typedef DCC_RETURN DCC_READ_PTR(DCCMemory *mem, uint32_t offset, uint32_t size, uint8_t *dest, uint32_t *dest_size);

static uint8_t compBuf[DCC_BUFFER_SIZE + 0x2000];
static uint8_t rawBuf[DCC_BUFFER_SIZE + 0x2000];
#ifdef DCC_TESTING
extern uint32_t DCC_COMPRESS_MEMCPY(uint32_t algo, uint32_t src_offset, uint32_t size, uint8_t *dest);
void *absolute_to_relative(void* ptr) { return ptr; };
#else
extern void *absolute_to_relative(void *ptr);
#endif

// dcc code
void dcc_main(uint32_t BaseAddress1, uint32_t BaseAddress2, uint32_t BaseAddress3) {
    DCCMemory mem[16] = { 0 };
    uint8_t mem_has_spare[16] = { 0 };
    uint32_t BUF_INIT[512];
    uint32_t dcc_init_offset = 0;
    uint32_t ext_mem;
    Driver *devBase;
    DCC_RETURN res;

    for (int i = 0; i < 16; i++) {
        if (!devices[i].driver) break;
        devBase = (Driver *)absolute_to_relative(devices[i].driver);
        res = ((DCC_INIT_PTR *)absolute_to_relative(devBase->initialize))(&mem[i], devices[i].base_offset);
        if (res != DCC_OK) mem[i].type = MEMTYPE_NONE;

        switch (mem[i].type) {
            case MEMTYPE_NOR:
            case MEMTYPE_SUPERAND:
                ext_mem = DCC_MEM_EXTENDED(1, mem[i].page_size, mem[i].block_size, mem[i].size >> 20);
                mem_has_spare[i] = 0;
            WRITE_EXTMEM:
                BUF_INIT[dcc_init_offset++] = DCC_MEM_OK | (ext_mem << 16);
                BUF_INIT[dcc_init_offset++] = mem[i].manufacturer | (mem[i].device_id << 16);
                BUF_INIT[dcc_init_offset++] = ext_mem;
                break;

            case MEMTYPE_NAND:
                BUF_INIT[dcc_init_offset++] = DCC_MEM_OK | (mem[i].page_size << 16);
                BUF_INIT[dcc_init_offset++] = mem[i].manufacturer | (mem[i].device_id << 16);
                mem_has_spare[i] = 1;
                break;

            case MEMTYPE_ONENAND:
            case MEMTYPE_AND:
            case MEMTYPE_AG_AND:
                ext_mem = DCC_MEM_EXTENDED(0, mem[i].page_size, mem[i].block_size, mem[i].size >> 20);
                mem_has_spare[i] = 1;
                goto WRITE_EXTMEM;

            default:
                BUF_INIT[dcc_init_offset++] = DCC_MEM_OK | (DCC_MEM_NONE << 16);
                BUF_INIT[dcc_init_offset++] = 0;
                mem_has_spare[i] = 0;

        }
    }

    BUF_INIT[dcc_init_offset++] = DCC_MEM_OK | (DCC_MEM_BUFFER(0) << 16);
    BUF_INIT[dcc_init_offset++] = DCC_BUFFER_SIZE;
    
    DN_Packet_Send((uint8_t *)BUF_INIT, dcc_init_offset << 2);

    uint32_t dcc_comp_packet_size;
    uint32_t flashIndex;
    uint32_t srcOffset;
    uint32_t srcSize;
    uint32_t destSize;

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
                DN_Packet_Send((uint8_t *)BUF_INIT, dcc_init_offset << 2);
                break;
                
            case CMD_GETMEMSIZE:
                flashIndex = (cmd >> 8) & 0xff;
                if (flashIndex == 0) {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(0x21, 0));
                } else if (flashIndex < 0x11 && mem[flashIndex - 1].type != MEMTYPE_NONE) {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(0x21, mem[flashIndex - 1].size >> 20));
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
#ifndef DCC_TESTING
                    switch (algo) {
                        case CMD_READ_COMP_NONE:
                            dcc_comp_packet_size = DN_Packet_CompressNone((uint8_t *)srcOffset, srcSize, compBuf);
                            break;

                        case CMD_READ_COMP_RLE:
                            dcc_comp_packet_size = DN_Packet_Compress((uint8_t *)srcOffset, srcSize, compBuf);
                            break;

                        #if HAVE_MINILZO
                        case CMD_READ_COMP_LZO:
                            dcc_comp_packet_size = DN_Packet_Compress2((uint8_t *)srcOffset, srcSize, compBuf);
                            break;
                        #endif

                        #if HAVE_LZ4
                        case CMD_READ_COMP_LZ4:
                            dcc_comp_packet_size = DN_Packet_Compress3((uint8_t *)srcOffset, srcSize, compBuf);
                            break;
                        #endif

                        default:
                            DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_INVALID_ARGS));
                            continue;
                    }
#else
                    dcc_comp_packet_size = DCC_COMPRESS_MEMCPY(algo, srcOffset, srcSize, compBuf);
#endif
                    
                    DN_Packet_Send(compBuf, dcc_comp_packet_size);
                } else if (flashIndex < 0x11 && mem[flashIndex - 1].type != MEMTYPE_NONE) {
                    switch (mem[flashIndex - 1].type) {
                        case MEMTYPE_NAND:
                        case MEMTYPE_ONENAND:
                        case MEMTYPE_SUPERAND:
                        case MEMTYPE_AND:
                        case MEMTYPE_AG_AND:
                            devBase = (Driver *)absolute_to_relative(devices[flashIndex - 1].driver);
                            res = ((DCC_READ_PTR *)absolute_to_relative(devBase->read))(&mem[flashIndex - 1], srcOffset, srcSize, rawBuf, &destSize);
                            if (res != DCC_OK) {
                                DN_Packet_Send_One(CMD_READ_RESP_FAIL(res));
                                continue;
                            }
                            
                            switch (algo) {
                                case CMD_READ_COMP_NONE:
                                    dcc_comp_packet_size = DN_Packet_CompressNone(rawBuf, destSize, compBuf);
                                    break;

                                case CMD_READ_COMP_RLE:
                                    dcc_comp_packet_size = DN_Packet_Compress(rawBuf, destSize, compBuf);
                                    break;

                                #if HAVE_MINILZO
                                case CMD_READ_COMP_LZO:
                                    dcc_comp_packet_size = DN_Packet_Compress2(rawBuf, destSize, compBuf);
                                    break;
                                #endif

                                #if HAVE_LZ4
                                case CMD_READ_COMP_LZ4:
                                    dcc_comp_packet_size = DN_Packet_Compress3(rawBuf, destSize, compBuf);
                                    break;
                                #endif

                                default:
                                    DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_INVALID_ARGS));
                                    continue;
                            }

                            DN_Packet_Send(compBuf, dcc_comp_packet_size);
                            break;
                        case MEMTYPE_NOR:
                        default:
                            srcOffset += mem[flashIndex - 1].base_offset;
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

                if (flashIndex == 0) flashIndex = 1;

                if (flashIndex < 0x11 && mem[flashIndex - 1].type != MEMTYPE_NONE) {
                    // TODO: Erasing
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_WPROT_ERROR, flashIndex));
                } else {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_FLASH_NOENT, flashIndex));
                }
                break;

            case CMD_WRITE:
                uint32_t pAddrStart = DN_Packet_DCC_Read();
                uint32_t dataPackN = DN_Packet_DCC_Read();
                uint8_t progType = (cmd >> 8) & 0x7f;
                uint8_t useECC = (cmd >> 8) & 0x80;

                flashIndex = (cmd >> 16) & 0xff;

                if (flashIndex == 0) flashIndex = 1;

                if (flashIndex < 0x11 && mem[flashIndex - 1].type != MEMTYPE_NONE) {
                    if (dataPackN == CMD_WRITE_COMP_NONE) {
                        if (progType & 2) DN_Packet_Read(rawBuf, mem[flashIndex - 1].block_size);
                        if ((progType & 1) && mem_has_spare[flashIndex - 1]) DN_Packet_Read(rawBuf + mem[flashIndex - 1].block_size, mem[flashIndex - 1].block_size >> 5);
                    } else {
                        uint32_t comp_len = DN_Packet_DCC_Read();
                        DN_Packet_Read(compBuf, ALIGN4(comp_len));
                    }
                    uint32_t checksum = DN_Packet_DCC_Read();
                    // TODO: Writing
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_WPROT_ERROR, flashIndex));
                } else {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_FLASH_NOENT, flashIndex));
                }
                break;

            default:
                DN_Packet_Send_One(DCC_BAD_COMMAND(cmd & 0xff));
        }
    }
}