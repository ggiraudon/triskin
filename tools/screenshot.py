"""Save what the device is showing as a PNG, over its USB serial port.

Usage: python tools/screenshot.py [out.png] [--scale N]
Run it with PlatformIO's Python, which has pyserial:
    ~/.platformio/penv/Scripts/python tools/screenshot.py
"""
import argparse
import struct
import sys
import zlib

import serial
from serial.tools import list_ports

RASPBERRY_PI_VID = 0x2E8A


def find_port():
    for port in list_ports.comports():
        if port.vid == RASPBERRY_PI_VID:
            return port.device
    sys.exit("No Triskin serial port found. Is the board plugged in and running?")


def grab(port):
    """Returns (width, height, rows) with rows as lists of (r, g, b)."""
    with serial.Serial(port, 115200, timeout=3) as link:
        link.reset_input_buffer()
        link.write(b"shot\n")
        while True:
            line = link.readline()
            if not line:
                sys.exit("The device did not answer the 'shot' command.")
            if line.startswith(b"SHOT"):
                break
        fields = line.split()
        if fields[1] == b"ERR":
            sys.exit("The device could not take a screenshot (out of memory?).")
        width, height, stride = (int(f) for f in fields[1:4])
        raw = link.read(stride * height)
        if len(raw) != stride * height:
            sys.exit(f"Short read: got {len(raw)} of {stride * height} bytes.")
    rows = []
    for y in range(height):
        pixels = struct.unpack_from(f"<{width}H", raw, y * stride)
        rows.append([((p >> 11) * 255 // 31, ((p >> 5) & 63) * 255 // 63, (p & 31) * 255 // 31)
                     for p in pixels])
    return width, height, rows


def write_png(path, width, height, rows, scale):
    def chunk(kind, data):
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body))

    lines = bytearray()
    for row in rows:
        line = b"\x00" + b"".join(bytes(px) * scale for px in row)
        lines += line * scale
    with open(path, "wb") as out:
        out.write(b"\x89PNG\r\n\x1a\n")
        out.write(chunk(b"IHDR", struct.pack(">IIBBBBB", width * scale, height * scale, 8, 2, 0, 0, 0)))
        out.write(chunk(b"IDAT", zlib.compress(bytes(lines), 9)))
        out.write(chunk(b"IEND", b""))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("out", nargs="?", default="screenshot.png")
    parser.add_argument("--scale", type=int, default=1, help="integer upscale factor")
    args = parser.parse_args()
    width, height, rows = grab(find_port())
    write_png(args.out, width, height, rows, args.scale)
    print(f"wrote {args.out} ({width}x{height}, x{args.scale})")


if __name__ == "__main__":
    main()
