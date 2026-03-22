# pyright: reportUnknownMemberType=false, reportOptionalMemberAccess=false, reportPrivateUsage=false, reportUnknownVariableType=false, reportUninitializedInstanceVariable=false

from ctypes import *
from pylink import JLink
import crcmod
from enum import IntEnum
import struct
import random
import traceback
import sys
import time

from logging import Logger, getLogger, basicConfig
from dataclasses import dataclass
from typing_extensions import override
from construct import *
from construct_typed import DataclassMixin, DataclassStruct, csfield
from typing import IO, Any, Literal, Self
from tap import Tap

class _EnhancedDataclassMixin(DataclassMixin):
    @classmethod
    def format(cls):
        return DataclassStruct(cls)

    @classmethod
    def build(cls, obj: Self, **kw: Any):    # pyright: ignore[reportExplicitAny, reportAny]
        return cls.format().build(obj, **kw)

    @classmethod
    def parse(cls, data: bytes|bytearray, **kw: Any):# pyright: ignore[reportExplicitAny, reportAny]
        return cls.format().parse(data, **kw)

    @classmethod
    def parse_file(cls, file: str, **kw: Any):# pyright: ignore[reportExplicitAny, reportAny]
        return cls.format().parse_file(file, **kw)

    @classmethod
    def parse_stream(cls, stream: IO[bytes], **kw: Any):# pyright: ignore[reportExplicitAny, reportAny]
        return cls.format().parse_stream(stream, **kw)

class ArmRegs(IntEnum):
    ARM_REG_R0 = 0
    ARM_REG_R1 = 1
    ARM_REG_R2 = 2
    ARM_REG_R3 = 3
    ARM_REG_R4 = 4
    ARM_REG_R5 = 5
    ARM_REG_R6 = 6
    ARM_REG_R7 = 7
    ARM_REG_CPSR = 8
    ARM_REG_R15 = 9
    ARM_REG_R8_USR = 10
    ARM_REG_R9_USR = 11
    ARM_REG_R10_USR = 12
    ARM_REG_R11_USR = 13
    ARM_REG_R12_USR = 14
    ARM_REG_R13_USR = 15
    ARM_REG_R14_USR = 16
    ARM_REG_SPSR_FIQ = 17
    ARM_REG_R8_FIQ = 18
    ARM_REG_R9_FIQ = 19
    ARM_REG_R10_FIQ = 20
    ARM_REG_R11_FIQ = 21
    ARM_REG_R12_FIQ = 22
    ARM_REG_R13_FIQ = 23
    ARM_REG_R14_FIQ = 24
    ARM_REG_SPSR_SVC = 25
    ARM_REG_R13_SVC = 26
    ARM_REG_R14_SVC = 27
    ARM_REG_SPSR_ABT = 28
    ARM_REG_R13_ABT = 29
    ARM_REG_R14_ABT = 30
    ARM_REG_SPSR_IRQ = 31
    ARM_REG_R13_IRQ = 32
    ARM_REG_R14_IRQ = 33
    ARM_REG_SPSR_UND = 34
    ARM_REG_R13_UND = 35
    ARM_REG_R14_UND = 36
    ARM_REG_FPSID = 37
    ARM_REG_FPSCR = 38
    ARM_REG_FPEXC = 39
    ARM_REG_FPS0 = 40
    ARM_REG_FPS1 = 41
    ARM_REG_FPS2 = 42
    ARM_REG_FPS3 = 43
    ARM_REG_FPS4 = 44
    ARM_REG_FPS5 = 45
    ARM_REG_FPS6 = 46
    ARM_REG_FPS7 = 47
    ARM_REG_FPS8 = 48
    ARM_REG_FPS9 = 49
    ARM_REG_FPS10 = 50
    ARM_REG_FPS11 = 51
    ARM_REG_FPS12 = 52
    ARM_REG_FPS13 = 53
    ARM_REG_FPS14 = 54
    ARM_REG_FPS15 = 55
    ARM_REG_FPS16 = 56
    ARM_REG_FPS17 = 57
    ARM_REG_FPS18 = 58
    ARM_REG_FPS19 = 59
    ARM_REG_FPS20 = 60
    ARM_REG_FPS21 = 61
    ARM_REG_FPS22 = 62
    ARM_REG_FPS23 = 63
    ARM_REG_FPS24 = 64
    ARM_REG_FPS25 = 65
    ARM_REG_FPS26 = 66
    ARM_REG_FPS27 = 67
    ARM_REG_FPS28 = 68
    ARM_REG_FPS29 = 69
    ARM_REG_FPS30 = 70
    ARM_REG_FPS31 = 71
    ARM_REG_R8 = 72
    ARM_REG_R9 = 73
    ARM_REG_R10 = 74
    ARM_REG_R11 = 75
    ARM_REG_R12 = 76
    ARM_REG_R13 = 77
    ARM_REG_R14 = 78
    ARM_REG_SPSR = 79

class ResetType(IntEnum):
    ARM_RESET_TYPE_NORMAL = 0
    ARM_RESET_TYPE_BP0 = 1
    ARM_RESET_TYPE_ADI = 2
    ARM_RESET_TYPE_NO_RESET = 3
    ARM_RESET_TYPE_HALT_WP = 4
    ARM_RESET_TYPE_HALT_DBGRQ = 5
    ARM_RESET_TYPE_SOFT = 6
    ARM_RESET_TYPE_HALT_DURING = 7
    ARM_RESET_TYPE_SAM7 = 8
    ARM_RESET_TYPE_LPC = 9
    RESET_TYPE_CORE = 100
    RESET_TYPE_RESET_PIN = 101

@dataclass
class NANDDeviceInfo:
    page_size: int
    flash_size: int
    block_size: int
    bits: int

_NAND_DEV_IDS = {
    0x6e: NANDDeviceInfo(0x100, 0x100000, 0x800, 8),
    0x64: NANDDeviceInfo(0x100, 0x200000, 0x800, 8),
    0xe8: NANDDeviceInfo(0x100, 0x100000, 0x800, 8),
    0xec: NANDDeviceInfo(0x100, 0x100000, 0x800, 8),
    0xea: NANDDeviceInfo(0x100, 0x200000, 0x800, 8),
    0x6b: NANDDeviceInfo(0x200, 0x400000, 0x2000, 8),
    0xe3: NANDDeviceInfo(0x200, 0x400000, 0x2000, 8),
    0xe5: NANDDeviceInfo(0x200, 0x400000, 0x2000, 8),
    0xd6: NANDDeviceInfo(0x200, 0x800000, 0x2000, 8),
    0x39: NANDDeviceInfo(0x200, 0x8000000, 0x4000, 8),
    0xe6: NANDDeviceInfo(0x200, 0x800000, 0x2000, 8),
    0x49: NANDDeviceInfo(0x200, 0x8000000, 0x4000, 16),
    0x59: NANDDeviceInfo(0x200, 0x8000000, 0x4000, 16),
    0x33: NANDDeviceInfo(0x200, 0x1000000, 0x4000, 8),
    0x73: NANDDeviceInfo(0x200, 0x1000000, 0x4000, 8),
    0x43: NANDDeviceInfo(0x200, 0x1000000, 0x4000, 16),
    0x53: NANDDeviceInfo(0x200, 0x1000000, 0x4000, 16),
    0x35: NANDDeviceInfo(0x200, 0x2000000, 0x4000, 8),
    0x75: NANDDeviceInfo(0x200, 0x2000000, 0x4000, 8),
    0x45: NANDDeviceInfo(0x200, 0x2000000, 0x4000, 16),
    0x55: NANDDeviceInfo(0x200, 0x2000000, 0x4000, 16),
    0x36: NANDDeviceInfo(0x200, 0x4000000, 0x4000, 8),
    0x76: NANDDeviceInfo(0x200, 0x4000000, 0x4000, 8),
    0x46: NANDDeviceInfo(0x200, 0x4000000, 0x4000, 16),
    0x56: NANDDeviceInfo(0x200, 0x4000000, 0x4000, 16),
    0x78: NANDDeviceInfo(0x200, 0x8000000, 0x4000, 8),
    0x79: NANDDeviceInfo(0x200, 0x8000000, 0x4000, 8),
    0x72: NANDDeviceInfo(0x200, 0x8000000, 0x4000, 16),
    0x74: NANDDeviceInfo(0x200, 0x8000000, 0x4000, 16),
    0x71: NANDDeviceInfo(0x200, 0x10000000, 0x4000, 8),
    0xa2: NANDDeviceInfo(0x00, 0x4000000, 0x00, 8),
    0xb2: NANDDeviceInfo(0x00, 0x4000000, 0x00, 16),
    0xc2: NANDDeviceInfo(0x00, 0x4000000, 0x00, 16),
    0xf2: NANDDeviceInfo(0x00, 0x4000000, 0x00, 8),
    0xa1: NANDDeviceInfo(0x00, 0x8000000, 0x00, 8),
    0xb1: NANDDeviceInfo(0x00, 0x8000000, 0x00, 16),
    0xc1: NANDDeviceInfo(0x00, 0x8000000, 0x00, 16),
    0xf1: NANDDeviceInfo(0x00, 0x8000000, 0x00, 8),
    0xaa: NANDDeviceInfo(0x00, 0x10000000, 0x00, 8),
    0xba: NANDDeviceInfo(0x00, 0x10000000, 0x00, 16),
    0xca: NANDDeviceInfo(0x00, 0x10000000, 0x00, 16),
    0xda: NANDDeviceInfo(0x00, 0x10000000, 0x00, 8),
    0xac: NANDDeviceInfo(0x00, 0x20000000, 0x00, 8),
    0xbc: NANDDeviceInfo(0x00, 0x20000000, 0x00, 16),
    0xcc: NANDDeviceInfo(0x00, 0x20000000, 0x00, 16),
    0xdc: NANDDeviceInfo(0x00, 0x20000000, 0x00, 8),
    0xa3: NANDDeviceInfo(0x00, 0x40000000, 0x00, 8),
    0xb3: NANDDeviceInfo(0x00, 0x40000000, 0x00, 16),
    0xc3: NANDDeviceInfo(0x00, 0x40000000, 0x00, 16),
    0xd3: NANDDeviceInfo(0x00, 0x40000000, 0x00, 8),
    0xa5: NANDDeviceInfo(0x00, 0x80000000, 0x00, 8),
    0xb5: NANDDeviceInfo(0x00, 0x80000000, 0x00, 16),
    0xc5: NANDDeviceInfo(0x00, 0x80000000, 0x00, 16),
    0xd5: NANDDeviceInfo(0x00, 0x80000000, 0x00, 8)
}

@dataclass
class DCCLoaderInfoPacket(_EnhancedDataclassMixin):
    @dataclass
    class Error(_EnhancedDataclassMixin):
        error_code: int = csfield(Hex(Int32ul))

    @dataclass
    class BufferInfo(_EnhancedDataclassMixin):
        read_buffer_size: int = csfield(Int32ul)
        write_buffer_size: int = csfield(IfThenElse(this._.dev_type & 0x100, Int32ul, Computed(this.read_buffer_size)))

    @dataclass
    class MemoryInfo(_EnhancedDataclassMixin):
        @dataclass
        class ExtendParams(_EnhancedDataclassMixin):
            emmc_no_sectors: int | None = csfield(If(this._._dev_type & 0x10, Int32ul))
            name: str | None = csfield(If(this._._dev_type & 0x80, Aligned(4, PascalString(Int8ul, "ascii"))))
            extend_flags: int = csfield(Hex(Rebuild(Int8ul, this._._dev_type & 0xff)))
            page_size_log2: int = csfield(Hex(Int8ul))
            block_size_log2: int = csfield(Hex(Int8ul))
            size_mb_log2: int = csfield(Hex(Int8ul))

        _dev_type: int = csfield(Computed(this._.dev_type)) # pyright: ignore[reportAny]
        manufacturer_id: int = csfield(Hex(Int16ul))
        device_id: int = csfield(Hex(Int16ul))
        extend_params: ExtendParams | None = csfield(If((this._dev_type & 3) == 3, ExtendParams.format()))
        have_oob: bool = csfield(Computed(lambda x: (x._dev_type & 0xb) in [0x0, 0x3])) # pyright: ignore[reportAny]

    dev_magic: bytes = csfield(Const(b"OK"))
    dev_type: int = csfield(Hex(Int16ul))
    data: Error | BufferInfo | MemoryInfo = csfield(IfThenElse(this.dev_type == 0xffff, Error.format(), IfThenElse(this.dev_type & 4, BufferInfo.format(), MemoryInfo.format())))

@dataclass
class DCCMemDevice:
    manufacturer_id: int
    device_id: int
    page_size: int # 1 << n
    block_size: int # 1 << n
    flash_size: int # (1 << n) << 20
    have_oob: bool

class DumpDCC():
    def __init__(self, jlink: JLink, bp_loader: bool=False, logger: Logger | None=None):
        self.jlink: JLink = jlink # JLink context
        self.__is_bp_loader: bool = bp_loader # Breakpoint loader

        # Variables
        self.__is_loaded: bool = False 
        self.__start_offset: int = 0
        self.flash_devices: list[DCCMemDevice | None] = []
        self.__logger: Logger | None = logger
        self.__read_buf_size: int = 0
        self.__write_buf_size: int = 0
        self.__bp_loader_breakpoint_pc: int = 0
        self.__bp_loader_read_data: list[int] = []
        self.__bp_loader_data_fetched: bool = False

    def load(self, binary: bytes|bytearray|str, start_offset: int):
        if isinstance(binary, str):
            binary = open(binary, "rb").read()

        assert (start_offset % 4) == 0, "Invalid destination offset"

        # 00 - Reset variables
        self.__is_loaded = False
        self.__start_offset = start_offset
        self.flash_devices.clear()
        self.__read_buf_size = 0
        self.__write_buf_size = 0
        self.__bp_loader_breakpoint_pc = 0
        self.jlink.breakpoint_clear_all()
        self.__bp_loader_read_data.clear()
        self.__bp_loader_data_fetched = False

        # 01 - Ram self test
        if self.__logger is not None:
            self.__logger.debug("Running RAM self test for 0x%08x", start_offset)

        ram_selftest_data = random.randbytes(0x800)
        self.jlink.memory_write8(start_offset, [x for x in ram_selftest_data])

        ram_selftest_read = bytes(self.jlink.memory_read8(start_offset, 0x800))
        assert ram_selftest_read == ram_selftest_data, "Upload test failed"

        # 02 - Upload and restart
        if self.__logger is not None:
            self.__logger.debug("Uploading loader to 0x%08x", start_offset)

        self.jlink.memory_write8(start_offset, [x for x in binary])

        if self.__is_bp_loader:
            #bp_loader_breakpoint = int.from_bytes(binary[0x34:0x38], "little")
            bp_loader_breakpoint: int = self.jlink.memory_read32(start_offset + 0x34, 1)[0]
            if self.__logger is not None:
                self.__logger.debug("Setting loader breakpoint at 0x%08x", bp_loader_breakpoint)

            self.__bp_loader_breakpoint_pc = bp_loader_breakpoint
            self.jlink.hardware_breakpoint_set(bp_loader_breakpoint, arm=True)

        self.jlink.register_write(ArmRegs.ARM_REG_CPSR, 0xd3) # Disable all interrupts and go to supervisor mode
        self.jlink.register_write(ArmRegs.ARM_REG_R15, start_offset)

        self.jlink.restart()

        # 03 - DCC Load start
        if self.__logger is not None:
            self.__logger.debug("Began flash device probing")

        dcc_init_data = GreedyRange(DCCLoaderInfoPacket.format()).parse(self.__dcc_read_data())

        for flash_n, f in enumerate(dcc_init_data, start=1):
            if isinstance(f.data, DCCLoaderInfoPacket.Error):
                if self.__logger is not None:
                    self.__logger.warning("Probe failed for dev id %d (code 0x%02x)", flash_n, f.data.error_code)

                self.flash_devices.append(None)

            elif isinstance(f.data, DCCLoaderInfoPacket.BufferInfo):
                if self.__logger is not None:
                    self.__logger.debug("read buffer size: 0x%08x, write buffer size: 0x%08x", f.data.read_buffer_size, f.data.write_buffer_size)

                self.__read_buf_size = f.data.read_buffer_size
                self.__write_buf_size = f.data.write_buffer_size

            else:
                if f.data.extend_params is None:
                    device_id = f.data.device_id & 0xff
                    is_nor = (f.dev_type & 0x3) == 1

                    block_size = (1 << (f.dev_type >> 8)) if is_nor else _NAND_DEV_IDS[device_id].block_size
                    page_size = 0x200 if is_nor else _NAND_DEV_IDS[device_id].page_size

                    if not page_size:
                        device_id_high = f.data.device_id >> 8
                        page_size_bits = device_id_high & 3
                        block_size_bits = (device_id_high >> 4) & 3

                        page_size = 1 << (10 + page_size_bits)
                        block_size = (1 << (6 + block_size_bits)) << 10

                    self.flash_devices.append(DCCMemDevice(f.data.manufacturer_id, f.data.device_id, page_size, block_size, _NAND_DEV_IDS[device_id].flash_size, f.data.have_oob))

                else:
                    self.flash_devices.append(DCCMemDevice(f.data.manufacturer_id, f.data.device_id, 1 << f.data.extend_params.page_size_log2, 1 << f.data.extend_params.block_size_log2, (1 << f.data.extend_params.size_mb_log2) << 20, f.data.have_oob))
        
        if self.__logger is not None:
            self.__logger.debug("flash devices:")
            for id, fd in enumerate(self.flash_devices, start=1):
                self.__logger.debug("id %d: %s", id, fd)

        self.__is_loaded = True

    def __dcc_do_read(self, length: int) -> list[int]:
        if self.__is_bp_loader:
            if not self.__bp_loader_data_fetched:
                # 01 - Watit for breakpoint
                while True:
                    time.sleep(0.01)
                    if self.jlink.halted():
                        break
                
                # 02 - Check breakpoint PC
                pc = self.jlink.register_read(ArmRegs.ARM_REG_R15)
                assert pc == self.__bp_loader_breakpoint_pc, "Something is wrong with the breakpoint"

                # 03 - Read breakpoint data
                bp_loader_ptr: int = self.jlink.memory_read32(self.__start_offset + 0x30, 1)[0]
                bp_loader_size: int = self.jlink.memory_read32(self.__start_offset + 0x2c, 1)[0]

                self.__bp_loader_read_data = list[int](self.jlink.memory_read32(bp_loader_ptr, bp_loader_size >> 2)) # pyright: ignore[reportUnknownArgumentType]
                assert len(self.__bp_loader_read_data) == (bp_loader_size >> 2), f"dcc read error ({len(self.__bp_loader_read_data)} != {bp_loader_size >> 2})"

                # 04 - Mark as fetched
                self.__bp_loader_data_fetched = True

            if self.__logger is not None:
                self.__logger.debug("bp loader read buffer: %s, read length: %d", self.__bp_loader_read_data[:16], length)

            temp = self.__bp_loader_read_data[:length]
            self.__bp_loader_read_data = self.__bp_loader_read_data[length:]

            return temp

        else:
            dcc_read_data = (c_uint32 * length)()
            dcc_read_num: int = self.jlink._dll.JLINKARM_ReadDCC(dcc_read_data, length, 2500)

            assert dcc_read_num == length, f"dcc read error ({dcc_read_num} != {length})"
            
            return list[int](dcc_read_data)

    def __dcc_do_write(self, values: list[int]):
        if self.__is_bp_loader:
            self.__bp_loader_read_data.clear()
            self.__bp_loader_data_fetched = False

            bp_loader_ptr: int = self.jlink.memory_read32(self.__start_offset + 0x30, 1)[0]

            write_no_values = self.jlink.memory_write32(bp_loader_ptr, values)
            write_len = self.jlink.memory_write32(self.__start_offset + 0x2c, [len(values) << 2])

            assert write_no_values == len(values), f"dcc write error ({write_no_values} != {len(values)})"
            assert write_len == 1, f"dcc write error (len) ({write_len} != 1)"

        else:
            dcc_write_data = (c_uint32 * len(values))(*values)
            dcc_write_num: int = self.jlink._dll.JLINKARM_WriteDCC(dcc_write_data, len(values), 2500)
            
            assert dcc_write_num == len(values), f"dcc write error ({dcc_write_num} != {len(values)})"

    def __dcc_read_data(self) -> bytes:
        # 01 - Get Length
        dcc_data_len: int = self.__dcc_do_read(1)[0]

        # 02 - Read Data
        len_data_plus_crc: int = dcc_data_len + 1
        dcc_read_data: list[int] = self.__dcc_do_read(len_data_plus_crc)

        dcc_data: list[int] = dcc_read_data[:dcc_data_len]
        dcc_crc32_expect: int = dcc_read_data[dcc_data_len]
        dcc_data_bytes: bytes = b"".join(x.to_bytes(4, "little") for x in dcc_data)

        # 03 - Verify data
        dcc_crc32_hash = crcmod.mkCrcFun(0x104c11db7, 0xffffffff, False, 0)

        dcc_crc_computed = dcc_crc32_hash(dcc_data_bytes)
        assert dcc_crc_computed == dcc_crc32_expect, f"dcc data checksum mismatch (0x{dcc_crc_computed:08x} != 0x{dcc_crc32_expect:08x})"

        return dcc_data_bytes

    @staticmethod
    def __decompress(data: bytes) -> bytes:
        o = 0
        buf = bytearray()
        
        while o < len(data):
            c = int.from_bytes(data[o:o + 2], "little")

            if c & 0x8000:
                c &= 0x7fff
                #print("rle", hex(c[0]))
                buf += data[o + 2:o + 3] * c
                o += 3

            else:
                #print("raw", hex(c))
                buf += data[o + 2:(o + 2) + c]
                o += 2 + c

        return bytes(buf)

    def read_flash(self, offset: int, length: int, flash_id: int=1) -> tuple[bytes, bytes]:
        # 01 - Check parameters
        assert self.__is_loaded, "Please load the DCC loader code first"

        if flash_id == 0: # MCU
            assert (offset % 4) == 0, "read offset must be dword aligned"
            assert (length % 4) == 0, "read size must be dword aligned"

        else: # Flash
            read_flash_id = flash_id - 1
            assert read_flash_id < len(self.flash_devices) and self.flash_devices[read_flash_id] is not None, "Invalid flash ID"
            assert (offset % self.flash_devices[read_flash_id].page_size) == 0, "read offset must be page aligned"
            assert (length % self.flash_devices[read_flash_id].page_size) == 0, "read size must be page aligned"

        # 02 - Initialize variables
        outp_data = bytearray()
        outp_oob = bytearray()

        read_n = 0

        # 03 - Main read
        while length > 0:
            read_len = min(self.__read_buf_size, length)

            # 04 - Send read command
            if self.__logger is not None:
                self.__logger.debug("sending read command: id %d, offset: 0x%08x, size: 0x%08x", flash_id, offset + read_n, read_len)

            self.__dcc_do_write([0x52 | (flash_id << 8), offset + read_n, read_len])
            data = self.__dcc_read_data()

            # 05 - Handle response
            if self.__logger is not None:
                self.__logger.debug("got read response: %s", data[:16])

            match data[0]:
                case 0xff:
                    raise Exception(f"read flash error, code: 0x{data[1]:02x}")

                case 0x00:
                    outp_data += data[4:4+read_len]
                    outp_oob += data[4+read_len:]

                case 0x01:
                    rle_read_size = int.from_bytes(data[4:8], "little") - 4
                    rle_decomp = self.__decompress(data[8:8+rle_read_size])

                    outp_data += rle_decomp[:read_len]
                    outp_oob += rle_decomp[read_len:]

                case _:
                    raise Exception(f"not implemented: 0x{data[0]:02x}")

            read_n += read_len
            length -= read_len

        return bytes(outp_data), bytes(outp_oob)

def runHAS(j: JLink, has: str):
    myHAS = open(has, "rb")
    while True:
        fp = myHAS.read(4)
        if len(fp) < 4: break

        cmd = int.from_bytes(fp, "little", signed=True)
        if cmd == -1:
            offset, data = struct.unpack("<LL", myHAS.read(8))
            #print(f"{cmd} (WRITE): {hex(offset)} {hex(data)}")
            j.memory_write32(offset, data)

        elif cmd == -9:
            offset, data = struct.unpack("<LL", myHAS.read(8))
            #print(f"{cmd} (WRITE8): {hex(offset)} {hex(data)}")
            j.memory_write8(offset, data)

        elif cmd == -8:
            offset, data = struct.unpack("<LL", myHAS.read(8))
            #print(f"{cmd} (WRITE16): {hex(offset)} {hex(data)}")
            j.memory_write16(offset, data)

        elif cmd == -12: # COPROC
            cr_m, cr_n, cp_no, op, data = struct.unpack("<BBBBL", myHAS.read(8))            
            #print(F"{cmd} (COPROC) {cp_no}, {cr_n}, {cr_m}, {op}, {hex(data)}")
            if cp_no == 15: j.cp15_register_write(cr_n, op, cr_m, 0, data)

        else:
            raise Exception(f"command {cmd} {hex(myHAS.tell() - 4)}")

class Args(Tap):
    loader: str # DCC loader to load
    output: str # Output file
    load_offset: int # Load offset
    read_start_offset: int # Read start offset
    read_size: int # Read size (enter 0 for full flash)
    family: str = "ARM9" # Device family
    script: str = "" # HAS script
    speed: int = -2 # TCK speed (-2 = RTCK, -1 = Auto)
    flash_id: int = 1 # Read flash ID (0 = MCU)
    reset_strategy: Literal["halt_reset", "halt_bp", "halt_wp", "halt_dbgrq", "software"] = "halt_reset" # ARM Reset strategy
    breakpoint_loader: bool = False # Loader is a breakpoint based loader
    read_block_size: int = 0x100000 # DCC read block size
    
    @staticmethod
    def intorhex(x: str):
        try:
            return int(x)

        except ValueError:
            return int(x, 16)

    @override
    def configure(self) -> None:
        self.add_argument("loader")
        self.add_argument("output")
        self.add_argument("load_offset", type=self.intorhex)
        self.add_argument("read_start_offset", type=self.intorhex)
        self.add_argument("read_size", type=self.intorhex)
        self.add_argument("--read_block_size", type=self.intorhex)

if __name__ == "__main__":
    basicConfig(level="INFO", format="[%(levelname)s] (%(name)s) %(message)s")
    logger = getLogger("DCC Dumper")

    args = Args().parse_args()
    speed = "adaptive" if args.speed == -2 else ("auto" if args.speed == -1 else args.speed)
    logger.info("Loader: %s", args.loader)
    logger.info("Output: %s", args.output)
    logger.info("Load offset: 0x%08x", args.load_offset)
    logger.info("Read start offset: 0x%08x", args.read_start_offset)
    logger.info("Read size: 0x%08x", args.read_size)
    logger.info("Speed (kHZ): %s", speed)
    logger.info("Family: %s", args.family)
    logger.info("Script: %s", args.script if args.script else "(none)")
    logger.info("Target Flash ID: %d", args.flash_id)
    logger.info("Breakpoint loader: %s", args.breakpoint_loader)
    logger.info("Read block size: 0x%08x", args.read_block_size)

    myJLink: JLink = JLink()
    myJLink.disable_dialog_boxes()
    myJLink.exec_command("SuppressEmuUSBDialog")

    logger.info("Connecting to JLink")
    myJLink.open()

    logger.info("Connecting to target")
    myJLink.connect(args.family, speed=speed) # pyright: ignore[reportArgumentType]

    logger.info("Halting target")
    myJLink.halt()

    assert myJLink.halted(), "Target cannot be halted" # Cannot halt the target

    logger.info("Resetting target")
    match args.reset_strategy:
        case "halt_reset":
            myJLink.set_reset_strategy(ResetType.ARM_RESET_TYPE_NORMAL)

        case "halt_bp":
            myJLink.set_reset_strategy(ResetType.ARM_RESET_TYPE_BP0)

        case "halt_wp":
            myJLink.set_reset_strategy(ResetType.ARM_RESET_TYPE_HALT_WP)

        case "halt_dbgrq":
            myJLink.set_reset_strategy(ResetType.ARM_RESET_TYPE_HALT_DBGRQ)

        case "software":
            myJLink.set_reset_strategy(ResetType.ARM_RESET_TYPE_SOFT)

    myJLink.reset()
    assert myJLink.halted(), "Target not halted after reset" # Cannot halt the target after reset

    if args.script: 
        logger.info("Executing HAS script")
        runHAS(myJLink, args.script)

    logger.info("Uploading loader to target")
    loader = DumpDCC(myJLink, args.breakpoint_loader, logger)
    loader.load(args.loader, args.load_offset)

    logger.info("Available flash devices:")
    for id, flash in enumerate(filter(lambda x: x is not None, loader.flash_devices), start=1):
        logger.info("CHIP%d: ID 0x%02X/0x%02X (%dMB)", id, flash.manufacturer_id, flash.device_id, flash.flash_size >> 20)

    dump_oob = bytearray()

    with open(args.output, "wb") as dumpOut:
        try:
            cur_offset = args.read_start_offset
            END_OFFSET = args.read_start_offset + args.read_size

            while cur_offset < END_OFFSET:
                read_size = min(args.read_block_size, END_OFFSET - cur_offset)
                logger.debug("Reading %d bytes command to loader at 0x%08x", read_size, cur_offset)

                blk_dump_data, blk_dump_oob = loader.read_flash(cur_offset, read_size, args.flash_id)

                dumpOut.write(blk_dump_data)
                dump_oob += blk_dump_oob

                cur_offset += read_size

            dumpOut.write(dump_oob)

        except Exception as e:
            print(f"ERROR: {e}", file=sys.stderr)
            traceback.print_exc()

            try:
                myJLink.halt()
                assert myJLink.halted(), "cannot halt"

                print("registers:")

                regs = myJLink.register_list()
                vals = myJLink.register_read_multiple(regs)

                for reg, val in zip(regs, vals):
                    name = myJLink.register_name(reg)
                    print(f"{name} = 0x{val:08x}")

                print("stack (r13; sp:sp+0x40):")
                sp = myJLink.register_read(ArmRegs.ARM_REG_R13_SVC)
                for e, i in enumerate(myJLink.memory_read32(sp, 16)):
                    print(f"0x{sp + (4 * e):08x} = 0x{i:08x}")

                print("stack (r13; sp-0x40:sp):")
                for e, i in enumerate(myJLink.memory_read32(sp - 0x40, 16)):
                    print(f"0x{(sp - 0x40) + (4 * e):08x} = 0x{i:08x}")

            except Exception as e:
                print(f"cannot get arm state dump:", file=sys.stderr)
                traceback.print_exc()

    myJLink.close()
