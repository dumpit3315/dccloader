#include "dcc/dn_dcc_proto.h"
#include "dcc/plat.h"
#include "devices.h"

typedef DCC_RETURN DCC_INIT_PTR(DCCMemory *mem, uint32_t offset);
typedef DCC_RETURN DCC_READ_PTR(DCCMemory *mem, uint32_t offset, uint32_t size, uint8_t *dest, uint32_t *dest_size);

#ifdef CDEFS
const char *CFLAGS = "C:DumpNow DCC Loader. (c) 2025 Wrapper. Compile flags: " CDEFS;
#endif

static uint8_t rawBuf[DCC_BUFFER_SIZE + 0x2000];
#if HAVE_LZ4 || HAVE_MINILZO
static uint8_t compBuf[DCC_BUFFER_SIZE + 0x4000];
#endif
#ifdef DCC_TESTING
extern void DCC_COMPRESS_MEMCPY(uint32_t algo, uint32_t src_offset, uint32_t size);
void *absolute_to_relative(void* ptr) { return ptr; };
#else
extern void *absolute_to_relative(void *ptr);
#endif

size_t strlen(const char *str);

// dcc code
void dcc_main(uint32_t StartAddress, uint32_t PageSize) {
    DCCMemory mem[16] = { 0 };
    uint8_t mem_has_spare[16] = { 0 };
    uint32_t BUF_INIT[2048];
    uint32_t dcc_init_offset = 0;
    uint32_t ext_mem;
    Driver *devBase;
    DCC_RETURN res;

    /* 01 - Probe flash devices */
    for (int i = 0; i < 16; i++) {
        if (!devices[i].driver) break; // Break when reaching the end of list

        /* Probe device */
        devBase = (Driver *)absolute_to_relative(devices[i].driver);
        res = ((DCC_INIT_PTR *)absolute_to_relative(devBase->initialize))(&mem[i], devices[i].base_offset);
        if (res != DCC_OK) mem[i].type = MEMTYPE_NONE;

        /* Print appropriate value */
        switch (mem[i].type) {
            /* Anything without spare */
            case MEMTYPE_NOR:
            case MEMTYPE_SUPERAND:
                ext_mem = DCC_MEM_EXTENDED(1, mem[i].page_size, mem[i].block_size, mem[i].size >> 20);
                mem_has_spare[i] = 0;
            /* Extended memory logic */
            WRITE_EXTMEM:
                /* Set name flag if memory name is defined */
                if (strlen(mem[i].name)) ext_mem |= 0x80;

                /* First device info */
                BUF_INIT[dcc_init_offset++] = DCC_MEM_OK | (ext_mem << 16);
                BUF_INIT[dcc_init_offset++] = mem[i].manufacturer | (mem[i].device_id << 16);
                
                /* Print additional information */
                if (strlen(mem[i].name)) {
                    int sLen = strlen(mem[i].name);
                    uint8_t *bufCast = (uint8_t *)BUF_INIT;
                    
                    /* 8-bit length then name (WORD aligned) */
                    BUF_INIT[dcc_init_offset] = sLen;
                    INT_MEMCPY((bufCast + (dcc_init_offset << 2) + 1), mem[i].name, sLen);

                    dcc_init_offset += ALIGN4(1 + sLen) >> 2;
                }

                /* Second device info */
                BUF_INIT[dcc_init_offset++] = ext_mem;
                break;

            /* Regular NAND */
            case MEMTYPE_NAND:
                if (strlen(mem[i].name)) goto NAND_EXTMEM; // Extended device info if we have additional information

                /* Device info */
                BUF_INIT[dcc_init_offset++] = DCC_MEM_OK | (mem[i].page_size << 16);
                BUF_INIT[dcc_init_offset++] = mem[i].manufacturer | (mem[i].device_id << 16);
                mem_has_spare[i] = 1;
                break;

            /* Anything with spare */
            case MEMTYPE_ONENAND:
            case MEMTYPE_AND:
            case MEMTYPE_AG_AND:
            NAND_EXTMEM:
                ext_mem = DCC_MEM_EXTENDED(0, mem[i].page_size, mem[i].block_size, mem[i].size >> 20);
                mem_has_spare[i] = 1;
                goto WRITE_EXTMEM;

            /* When device probe fails, it goes here */
            default:
                BUF_INIT[dcc_init_offset++] = DCC_MEM_OK | (DCC_MEM_NONE << 16);
                BUF_INIT[dcc_init_offset++] = 0;
                mem_has_spare[i] = 0;

        }
    }

    /* 02 - Print buffer size */
    BUF_INIT[dcc_init_offset++] = DCC_MEM_OK | (DCC_MEM_BUFFER(0) << 16);
    BUF_INIT[dcc_init_offset++] = DCC_BUFFER_SIZE;
    
    DN_Packet_Send((uint8_t *)BUF_INIT, dcc_init_offset << 2);

    #if HAVE_LZ4 || HAVE_MINILZO
    uint32_t dcc_comp_packet_size;
    #endif
    uint32_t flashIndex;
    uint32_t srcOffset;
    uint32_t srcSize;
    uint32_t destSize;

    /* 03 - The loop */
    while (1) {
        wdog_reset();
        uint32_t cmd = DN_Packet_DCC_Read();

        switch (cmd & 0xff) {
            /* Settings */
            case CMD_CONFIGURE:
                for (int c = 0; c < (cmd >> 0x10); c += 4) {
                    DN_Packet_DCC_Read();
                }
                DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(0x38, 0x6));
                break;

            /* Get devices information */
            case CMD_GETINFO:
                DN_Packet_Send((uint8_t *)BUF_INIT, dcc_init_offset << 2);
                break;
                
            /* Get memory size */
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

            /* Flash read */
            case CMD_READ:
                srcOffset = DN_Packet_DCC_Read();
                srcSize = DN_Packet_DCC_Read();
                flashIndex = (cmd >> 8) & 0xff;
                uint8_t algo = (cmd >> 24) & 0xff;

                /* Check for read size not exceeding buffer */
                if (srcSize > DCC_BUFFER_SIZE) {
                    DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_INVALID_ARGS));
                    continue;
                }

                if (flashIndex == 0) { // Direct read
                Jump_Read_NOR:
#ifndef DCC_TESTING
                    switch (algo) {
                        case CMD_READ_COMP_NONE:
                            DN_Packet_WriteDirect((uint8_t *)srcOffset, srcSize);
                            break;

                        case CMD_READ_COMP_RLE:
                            DN_Packet_WriteDirectCompressed((uint8_t *)srcOffset, srcSize);
                            break;

                        #if HAVE_MINILZO
                        case CMD_READ_COMP_LZO:
                            dcc_comp_packet_size = DN_Packet_Compress2((uint8_t *)srcOffset, srcSize, compBuf);
                            DN_Packet_Send(compBuf, dcc_comp_packet_size);
                            break;
                        #endif

                        #if HAVE_LZ4
                        case CMD_READ_COMP_LZ4:
                            dcc_comp_packet_size = DN_Packet_Compress3((uint8_t *)srcOffset, srcSize, compBuf);
                            DN_Packet_Send(compBuf, dcc_comp_packet_size);
                            break;
                        #endif

                        default:
                            DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_INVALID_ARGS));
                            continue;
                    }
#else
                    DCC_COMPRESS_MEMCPY(algo, srcOffset, srcSize);
#endif
                    
                    // DN_Packet_Send(compBuf, dcc_comp_packet_size);
                } else if (flashIndex < 0x11 && mem[flashIndex - 1].type != MEMTYPE_NONE) {
                    switch (mem[flashIndex - 1].type) {
                        case MEMTYPE_NAND:
                        case MEMTYPE_ONENAND:
                        case MEMTYPE_SUPERAND:
                        case MEMTYPE_AND:
                        case MEMTYPE_AG_AND:
                            /* Get driver routines */
                            devBase = (Driver *)absolute_to_relative(devices[flashIndex - 1].driver);
                            res = ((DCC_READ_PTR *)absolute_to_relative(devBase->read))(&mem[flashIndex - 1], srcOffset, srcSize, rawBuf, &destSize);
                            if (res != DCC_OK) { // Check if error
                                DN_Packet_Send_One(CMD_READ_RESP_FAIL(res));
                                continue;
                            }
                            
                            /* Compression */
                            switch (algo) {
                                case CMD_READ_COMP_NONE:
                                    DN_Packet_WriteDirect(rawBuf, destSize);
                                    //dcc_comp_packet_size = DN_Packet_CompressNone(rawBuf, destSize, compBuf);
                                    break;

                                case CMD_READ_COMP_RLE:
                                    DN_Packet_WriteDirectCompressed(rawBuf, destSize);
                                    //dcc_comp_packet_size = DN_Packet_Compress(rawBuf, destSize, compBuf);
                                    break;

                                #if HAVE_MINILZO
                                case CMD_READ_COMP_LZO:
                                    dcc_comp_packet_size = DN_Packet_Compress2(rawBuf, destSize, compBuf);
                                    DN_Packet_Send(compBuf, dcc_comp_packet_size);
                                    break;
                                #endif

                                #if HAVE_LZ4
                                case CMD_READ_COMP_LZ4:
                                    dcc_comp_packet_size = DN_Packet_Compress3(rawBuf, destSize, compBuf);
                                    DN_Packet_Send(compBuf, dcc_comp_packet_size);
                                    break;
                                #endif

                                default:
                                    DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_INVALID_ARGS));
                                    continue;
                            }

                            // DN_Packet_Send(compBuf, dcc_comp_packet_size);
                            break;
                        case MEMTYPE_NOR:
                        default:
                            /* NOR reads directly */
                            srcOffset &= (mem[flashIndex - 1].size - 1);
                            srcOffset += mem[flashIndex - 1].base_offset;
                            goto Jump_Read_NOR;
                    }
                } else {
                    /* No flash found */
                    DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_FLASH_NOENT));
                }

                break;
            
            /* Flash erase */
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

            /* Flash write */
            case CMD_WRITE:
                flashIndex = (cmd >> 16) & 0xff;

                uint32_t pAddrStart = DN_Packet_DCC_Read();
                uint32_t dataPackN = DN_Packet_DCC_Read();
                uint8_t progType = (cmd >> 8) & 0x7f;
                uint8_t useECC = (cmd >> 8) & 0x80;
                uint32_t checksum_comp = 0xffffffff;

                if (flashIndex == 0) flashIndex = 1;

                if (flashIndex < 0x11 && mem[flashIndex - 1].type != MEMTYPE_NONE) {
                    if (dataPackN == CMD_WRITE_COMP_NONE) {
                        if (progType & 2) {
                            DN_Packet_Read(rawBuf, mem[flashIndex - 1].block_size);
                            checksum_comp = DN_Calculate_CRC32(checksum_comp, rawBuf, mem[flashIndex - 1].block_size);
                        }
                        if ((progType & 1) && mem_has_spare[flashIndex - 1]) {
                            DN_Packet_Read(rawBuf + ((progType & 2) ? mem[flashIndex - 1].block_size : 0), mem[flashIndex - 1].block_size >> 5);
                            checksum_comp = DN_Calculate_CRC32(checksum_comp, rawBuf + ((progType & 2) ? mem[flashIndex - 1].block_size : 0), mem[flashIndex - 1].block_size >> 5);
                        }
                    } else {
                        uint32_t comp_len = DN_Packet_DCC_Read();
                        DN_Packet_DCC_ReadCompressed(rawBuf, comp_len);
                        checksum_comp = DN_Calculate_CRC32(checksum_comp, rawBuf, comp_len);
                    }
                    uint32_t checksum = DN_Packet_DCC_Read();

                    if (checksum != checksum_comp) {
                        DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_CHECKSUM_ERROR, flashIndex));
                        continue;
                    }
                    // TODO: Writing
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_WPROT_ERROR, flashIndex));
                } else {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_FLASH_NOENT, flashIndex));
                }
                break;

            /* Catch-all for unknown commands */
            default:
                DN_Packet_Send_One(DCC_BAD_COMMAND(cmd & 0xff));
        }
    }
}