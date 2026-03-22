#include "dcc/dn_dcc_proto.h"
#include "dcc/plat.h"
#include "devices.h"

#ifdef CDEFS
const char CFLAGS[] = "C:DumpNow DCC Loader. (c) 2026 Wrapper.;Compile flags: " CDEFS ";Compile Date: " __DATE__;
#endif

static uint8_t read_buffer[DCC_BUFFER_SIZE + 0x2000];
#if !USE_BREAKPOINTS && !DISABLE_COMPRESS
static uint8_t compress_buffer[DCC_BUFFER_SIZE + 0x4000];
#endif

#ifdef DCC_TESTING
extern void DCC_COMPRESS_MEMCPY(uint32_t algo, uint32_t src_offset, uint32_t size);
#endif

size_t strlen(const char *str);

static inline uint8_t dcc_send_data(uint8_t *data, uint32_t size, uint8_t algo) {
    uint32_t dcc_comp_packet_size;

    switch (algo) {
        case CMD_READ_COMP_RLE:
        #if !USE_BREAKPOINTS && !DISABLE_COMPRESS
            dcc_comp_packet_size = DN_Packet_Compress(data, size, compress_buffer);
            DN_Packet_Send(compress_buffer, dcc_comp_packet_size);
            break;
        #endif

        case CMD_READ_COMP_NONE:
            DN_Packet_Send_DirectUncompressed(data, size);
            break;
        
        #if HAVE_MINILZO && !USE_BREAKPOINTS && !DISABLE_COMPRESS
        case CMD_READ_COMP_LZO:
            dcc_comp_packet_size = DN_Packet_Compress2(data, size, compress_buffer);
            DN_Packet_Send(compress_buffer, dcc_comp_packet_size);
            break;
        #endif

        #if HAVE_LZ4 && !USE_BREAKPOINTS && !DISABLE_COMPRESS
        case CMD_READ_COMP_LZ4:
            dcc_comp_packet_size = DN_Packet_Compress3(data, size, compress_buffer);
            DN_Packet_Send(compress_buffer, dcc_comp_packet_size);
            break;
        #endif

        default:
            DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_INVALID_ARGS));
            return 0;
    }

    return 1;
}

// dcc code
void dcc_main(uint32_t StartAddress, uint32_t PageSize) {
    DCCMemory dev_mem[16] = { 0 };
    uint8_t mem_has_oob[16] = { 0 };
    uint32_t dcc_init_buf[2048];
    uint32_t dcc_init_offset = 0;
    uint32_t temp_ext_mem;
    DCC_RETURN res;

    /* 01 - Probe flash devices */
    for (int i = 0; i < 16; i++) {
        if (!devices[i].driver) break; // Break when reaching the end of list

        /* Probe device */
        res = devices[i].driver->initialize(&dev_mem[i], devices[i].base_offset, PageSize);
        if (res != DCC_OK) 
            dev_mem[i].type = MEMTYPE_NONE;

        /* Print appropriate value */
        switch (dev_mem[i].type) {
            /* Anything without spare */
            case MEMTYPE_NOR:
            case MEMTYPE_SUPERAND:
                temp_ext_mem = DCC_MEM_EXTENDED(1, dev_mem[i].page_size, dev_mem[i].block_size, dev_mem[i].size >> 20);
                mem_has_oob[i] = 0;

            /* Extended memory logic */
            WRITE_EXTMEM:
                /* Set name flag if memory name is defined */
                if (strlen(dev_mem[i].name)) temp_ext_mem |= 0x80;

                /* First device info */
                dcc_init_buf[dcc_init_offset++] = DCC_MEM_OK | (temp_ext_mem << 16);
                dcc_init_buf[dcc_init_offset++] = dev_mem[i].manufacturer | (dev_mem[i].device_id << 16);
                
                /* Print additional information */
                if (strlen(dev_mem[i].name)) {
                    int sLen = strlen(dev_mem[i].name);
                    uint8_t *dcc_init_buf_uint8 = (uint8_t *)dcc_init_buf;
                    
                    /* 8-bit length then name (WORD aligned) */
                    dcc_init_buf[dcc_init_offset] = sLen;
                    INT_MEMCPY((dcc_init_buf_uint8 + (dcc_init_offset << 2) + 1), dev_mem[i].name, sLen);

                    dcc_init_offset += ALIGN4(1 + sLen) >> 2;
                }

                /* Second device info */
                dcc_init_buf[dcc_init_offset++] = temp_ext_mem;
                break;

            /* Regular NAND */
            case MEMTYPE_NAND:
                if (strlen(dev_mem[i].name)) goto NAND_EXTMEM; // Extended device info if we have additional information

                /* Device info */
                dcc_init_buf[dcc_init_offset++] = DCC_MEM_OK | (dev_mem[i].page_size << 16);
                dcc_init_buf[dcc_init_offset++] = dev_mem[i].manufacturer | (dev_mem[i].device_id << 16);
                mem_has_oob[i] = 1;
                break;

            /* Anything with spare */
            case MEMTYPE_ONENAND:
            case MEMTYPE_AND:
            case MEMTYPE_AG_AND:
            NAND_EXTMEM:
                temp_ext_mem = DCC_MEM_EXTENDED(0, dev_mem[i].page_size, dev_mem[i].block_size, dev_mem[i].size >> 20);
                mem_has_oob[i] = 1;
                goto WRITE_EXTMEM;

            /* When device probe fails, it goes here */
            default:
                dcc_init_buf[dcc_init_offset++] = DCC_MEM_OK | (DCC_MEM_NONE << 16);
                dcc_init_buf[dcc_init_offset++] = dev_mem[i].probe_error_code;
                mem_has_oob[i] = 0;

        }
    }

    /* 02 - Print buffer size */
    dcc_init_buf[dcc_init_offset++] = DCC_MEM_OK | (DCC_MEM_BUFFER(0) << 16);
    dcc_init_buf[dcc_init_offset++] = DCC_BUFFER_SIZE;
    
    DN_Packet_Send((uint8_t *)dcc_init_buf, dcc_init_offset << 2);

    uint32_t fbFlashIndex;
    uint32_t fbStartOffset;
    uint32_t fbReadSize;
    uint32_t readDestSize;

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
                DN_Packet_Send((uint8_t *)dcc_init_buf, dcc_init_offset << 2);
                break;
                
            /* Get memory size */
            case CMD_GETMEMSIZE:
                fbFlashIndex = (cmd >> 8) & 0xff;
                if (fbFlashIndex == 0) {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(0x21, 0));
                } else if (fbFlashIndex < 0x11 && dev_mem[fbFlashIndex - 1].type != MEMTYPE_NONE) {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(0x21, dev_mem[fbFlashIndex - 1].size >> 20));
                } else {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_FLASH_NOENT, fbFlashIndex));
                }
                break;

            /* Flash read */
            case CMD_READ:
                fbStartOffset = DN_Packet_DCC_Read();
                fbReadSize = DN_Packet_DCC_Read();
                fbFlashIndex = (cmd >> 8) & 0xff;
                uint8_t compAlgo = (cmd >> 24) & 0xff;

                /* Check for read size not exceeding buffer */
                if (fbReadSize > DCC_BUFFER_SIZE) {
                    DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_INVALID_ARGS));
                    continue;
                }

                if (fbFlashIndex == 0) { // Direct read
                Jump_Read_NOR:
#ifndef DCC_TESTING
                    if (!dcc_send_data((uint8_t *)(fbStartOffset), fbReadSize, compAlgo))
                        continue;
#else
                    DCC_COMPRESS_MEMCPY(compAlgo, fbStartOffset, fbReadSize);
#endif
                    
                    // DN_Packet_Send(compBuf, dcc_comp_packet_size);
                } else if (fbFlashIndex < 0x11 && dev_mem[fbFlashIndex - 1].type != MEMTYPE_NONE) {
                    switch (dev_mem[fbFlashIndex - 1].type) {
                        case MEMTYPE_NAND:
                        case MEMTYPE_ONENAND:
                        case MEMTYPE_SUPERAND:
                        case MEMTYPE_AND:
                        case MEMTYPE_AG_AND:
                            /* Get driver routines */
                            res = devices[fbFlashIndex - 1].driver->read(&dev_mem[fbFlashIndex - 1], fbStartOffset, fbReadSize, read_buffer, &readDestSize);
                            if (res != DCC_OK) { // Check if error
                                DN_Packet_Send_One(CMD_READ_RESP_FAIL(res));
                                continue;
                            }
                            
                            /* Compression */
                            if (!dcc_send_data((uint8_t *)(read_buffer), readDestSize, compAlgo))
                                continue;

                            break;
                        case MEMTYPE_NOR:
                        default:
                            /* NOR reads directly */
                            fbStartOffset &= (dev_mem[fbFlashIndex - 1].size - 1);
                            fbStartOffset += dev_mem[fbFlashIndex - 1].base_offset;
                            goto Jump_Read_NOR;
                    }
                } else {
                    /* No flash found */
                    DN_Packet_Send_One(CMD_READ_RESP_FAIL(DCC_FLASH_NOENT));
                }

                break;
            
            /* Flash erase */
            case CMD_ERASE:
                fbStartOffset = DN_Packet_DCC_Read();
                fbReadSize = DN_Packet_DCC_Read();
                fbFlashIndex = (cmd >> 8) & 0xff;

                if (fbFlashIndex == 0) fbFlashIndex = 1;

                if (fbFlashIndex < 0x11 && dev_mem[fbFlashIndex - 1].type != MEMTYPE_NONE) {
                    // TODO: Erasing
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_WPROT_ERROR, fbFlashIndex));
                } else {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_FLASH_NOENT, fbFlashIndex));
                }
                break;

            /* Flash write */
            case CMD_WRITE:
                fbFlashIndex = (cmd >> 16) & 0xff;

                uint32_t pAddrStart = DN_Packet_DCC_Read();
                uint32_t dataPackN = DN_Packet_DCC_Read();
                uint8_t progType = (cmd >> 8) & 0x7f;
                uint8_t useECC = (cmd >> 8) & 0x80;
                uint32_t checksum_comp = 0xffffffff;

                if (fbFlashIndex == 0) fbFlashIndex = 1;

                if (fbFlashIndex < 0x11 && dev_mem[fbFlashIndex - 1].type != MEMTYPE_NONE) {
                    if (dataPackN == CMD_WRITE_COMP_NONE) {
                        if (progType & 2) {
                            DN_Packet_Read(read_buffer, dev_mem[fbFlashIndex - 1].block_size);
                            checksum_comp = DN_Calculate_CRC32(checksum_comp, read_buffer, dev_mem[fbFlashIndex - 1].block_size);
                        }
                        if ((progType & 1) && mem_has_oob[fbFlashIndex - 1]) {
                            DN_Packet_Read(read_buffer + ((progType & 2) ? dev_mem[fbFlashIndex - 1].block_size : 0), dev_mem[fbFlashIndex - 1].block_size >> 5);
                            checksum_comp = DN_Calculate_CRC32(checksum_comp, read_buffer + ((progType & 2) ? dev_mem[fbFlashIndex - 1].block_size : 0), dev_mem[fbFlashIndex - 1].block_size >> 5);
                        }
                    } else {
                        uint32_t comp_len = DN_Packet_DCC_Read();
                        DN_Packet_DCC_ReadCompressed(read_buffer, comp_len);
                        checksum_comp = DN_Calculate_CRC32(checksum_comp, read_buffer, comp_len);
                    }
                    uint32_t checksum = DN_Packet_DCC_Read();

                    if (checksum != checksum_comp) {
                        DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_CHECKSUM_ERROR, fbFlashIndex));
                        continue;
                    }
                    // TODO: Writing
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_WPROT_ERROR, fbFlashIndex));
                } else {
                    DN_Packet_Send_One(CMD_WRITE_ERASE_STATUS(DCC_FLASH_NOENT, fbFlashIndex));
                }
                break;

            /* Catch-all for unknown commands */
            default:
                DN_Packet_Send_One(DCC_BAD_COMMAND(cmd & 0xff));
        }
    }
}