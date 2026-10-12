# PlatformIO pre-script: point uploads at the Triskin's own serial port.
#
# With the HID mouse enabled the board's USB product ID differs from the ones
# PlatformIO knows for a Pico 2, so its auto-detection would fall back to any
# other serial port on the machine and pulse that one to request a reset.
Import("env")  # noqa: F821  (provided by PlatformIO)

from serial.tools import list_ports

RASPBERRY_PI_VID = 0x2E8A

ports = [p.device for p in list_ports.comports() if p.vid == RASPBERRY_PI_VID]
if ports:
    env.Replace(UPLOAD_PORT=ports[0])  # noqa: F821
else:
    # No running Triskin: either it is already in BOOTSEL mode (picotool will
    # find it) or it is unplugged. Either way there is no port to reset, and
    # an unrelated one must not be touched.
    env.BoardConfig().update("upload.use_1200bps_touch", False)  # noqa: F821
