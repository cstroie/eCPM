# eCPM - Embedded CP/M Machine

eCPM is a CP/M 2.2 emulator specifically designed for the ESP8266 microcontroller and other Arduino-compatible platforms. Inspired by the amazing [RunCPM](https://github.com/MockbaTheBorg/RunCPM) project, eCPM provides a lightweight implementation that allows you to run classic CP/M software on modern microcontrollers.

## Features

- **CP/M 2.2 Compatibility**: Full implementation of CP/M 2.2 BIOS and BDOS
- **Native File System**: Direct SD card file access without disk images
- **Multiple Platforms**: Primarily designed for ESP8266, but compatible with other Arduino devices
- **Wireless Connectivity**: Leverages ESP8266's WiFi capabilities for remote access
- **Flexible Memory Options**: Supports both on-chip RAM and SPI RAM expansion
- **Real Hardware Integration**: Direct access to serial console, printer, and other peripherals

## Architecture

Unlike traditional CP/M emulators that simulate an entire computer system, eCPM takes a different approach by directly implementing the CP/M operating system components:

- **8080 CPU Core**: Custom Intel 8080 emulator written in C++
- **BIOS**: Completely reimplemented in C++ with "hooks" to interface with the 8-bit environment
- **BDOS**: Full CP/M 2.2 Basic Disk Operating System implementation
- **CCP**: Uses the original Digital Research CCP (Console Command Processor)
- **File System**: Native SD card access organized in drive/user directory structure

## Hardware Requirements

### Minimum Requirements
- ESP8266 microcontroller (NodeMCU, Wemos D1 Mini, etc.)
- SD card module (SPI interface)
- Serial connection for console I/O

### Recommended Setup
- ESP8266 with 4MB flash
- MicroSD card module
- USB-to-Serial adapter for programming

## Memory Configuration

The ESP8266 has approximately 80KB of usable RAM, but WiFi connectivity reduces available memory to about 48KB. eCPM supports multiple memory configurations:

- **On-chip RAM**: Fastest option, limited to available DRAM
- **SPI RAM**: Extended memory support (slower but larger)
- **Buffered RAM**: Optimized access patterns for better performance

## File System Structure

eCPM organizes files on the SD card using a drive/user directory structure:

```
/eCPM/           # Base directory
  /A/            # Drive A
    /0/          # User 0
    /1/          # User 1
    ...
    /F/          # User 15
  /B/            # Drive B
    /0/          # User 0
    ...
```

This eliminates the need for traditional CP/M disk images while maintaining compatibility.

## Supported Applications

eCPM can run a wide variety of classic CP/M software including:

- **Text Editors**: WordStar, ED, ZDE
- **Programming Languages**: MBASIC, Z80ASM, L80
- **Utilities**: DDT, STAT, PIP
- **Games**: Text adventures and simple arcade games
- **Business Software**: dBase, SuperCalc (with memory constraints)

## Building and Installation

1. Install the Arduino IDE
2. Add ESP8266 board support
3. Install required libraries:
   - SD library
   - SPI library
4. Configure `config.h` for your hardware setup
5. Upload the CCP binary to your SD card
6. Compile and upload eCPM to your ESP8266

## Configuration Options

The `config.h` file allows customization of:

- Debug output levels
- Memory configuration (SPI RAM vs MCU RAM)
- Block size settings
- Buffer sizes
- Serial communication speed
- LED behavior

## Performance Considerations

- CPU emulation runs at native ESP8266 speeds
- SD card I/O is the primary performance bottleneck
- WiFi usage reduces available memory
- SPI RAM provides more memory but at reduced speed

## Related Projects

eCPM draws inspiration from several excellent projects:
- [RunCPM](https://github.com/MockbaTheBorg/RunCPM) - Primary inspiration
- cpm8266 - ESP8266 CP/M implementation
- Z80pack - Z80-based CP/M system
- Altair8800 - Arduino-based Altair 8800 replica
- IMSAI 8080 - ESP32-based IMSAI 8080 replica

## Limitations

- CP/M 2.2 only (no CP/M 3 or MP/M support)
- Memory constraints on ESP8266 limit large application usage
- No hardware-level peripheral emulation
- SD card access speed affects overall performance

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

This project stands on the shoulders of giants. Special thanks to:
- The RunCPM community for inspiration
- Alexander Demin for the 8080 core implementation
- All contributors to the various CP/M preservation projects

*This README was typed on eCPM, with WordStar 3.3, VT100 patched.*
