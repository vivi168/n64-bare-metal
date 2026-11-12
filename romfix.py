#!/usr/bin/env python3
from __future__ import annotations

import argparse
import struct
import zlib
from pathlib import Path

CRC_BOOTCODE_TO_CIC = {
    0x6170A4A1: 6101,
    0x90BB6CB5: 6102,
    0x0B050EE0: 6103,
    0x98BC2C86: 6105,
    0xACC8580A: 6106,
}
SEEDS = {
    6101: 0xF8CA4DDC,
    6102: 0xF8CA4DDC,
    6103: 0xA3886759,
    6105: 0xDF26F436,
    6106: 0x1FEA617A
}
CHECKSUM_END = 0x101000


def read_u32(data: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from(">I", data, offset)[0]


def rol32(value: int, amount: int) -> int:
    amount &= 31

    return ((value << amount) | (value >> ((32 - amount) & 31))) & 0xFFFFFFFF


def detect_cic(rom: bytearray) -> int:
    boot_crc = zlib.crc32(rom[0x40:0x1000]) & 0xFFFFFFFF

    try:
        return CRC_BOOTCODE_TO_CIC[boot_crc]
    except KeyError as error:
        raise ValueError(f"unknown IPL3 CRC32: {boot_crc:08X}") from error


def calculate_crc(rom: bytearray, cic: int) -> tuple[int, int]:
    t1 = t2 = t3 = t4 = t5 = t6 = SEEDS[cic]

    for offset in range(0x1000, CHECKSUM_END, 4):
        value = read_u32(rom, offset)

        if ((t6 + value) & 0xFFFFFFFF) < t6:
            t4 = (t4 + 1) & 0xFFFFFFFF

        t6 = (t6 + value) & 0xFFFFFFFF
        t3 ^= value
        rotated = rol32(value, value)
        t5 = (t5 + rotated) & 0xFFFFFFFF

        if t2 > value:
            t2 ^= rotated
        else:
            t2 ^= t6 ^ value

        if cic == 6105:
            t1 = (t1 + (read_u32(rom, 0x750 + (offset & 0xFF)) ^ value)) & 0xFFFFFFFF
        else:
            t1 = (t1 + (t5 ^ value)) & 0xFFFFFFFF

    if cic == 6103:
        return ((t6 ^ t4) + t3) & 0xFFFFFFFF, ((t5 ^ t2) + t1) & 0xFFFFFFFF

    if cic == 6106:
        return ((t6 * t4) + t3) & 0xFFFFFFFF, ((t5 * t2) + t1) & 0xFFFFFFFF

    return t6 ^ t4 ^ t3, t5 ^ t2 ^ t1


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path, help="N64 ROM to fix (in place)")
    args = parser.parse_args()

    rom = bytearray(args.rom.read_bytes())

    if len(rom) < 0x1000:
        raise ValueError("ROM does not contain a complete N64 header and IPL3")

    if len(rom) < CHECKSUM_END:
        rom.extend(b"\0" * (CHECKSUM_END - len(rom)))

    cic = detect_cic(rom)
    crc1, crc2 = calculate_crc(rom, cic)
    struct.pack_into(">II", rom, 0x10, crc1, crc2)
    args.rom.write_bytes(rom)

    print(f"{args.rom}: CIC-{cic}, CRC1={crc1:08X}, CRC2={crc2:08X}")


if __name__ == "__main__":
    main()
