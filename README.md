# MR-1 OpenPendant Firmware

This repository contains the firmware source code for the MR-1 OpenPendant, an ESP32-based pendant device designed for GRBL-controlled CNC machines. The project is primarily tested with the MR-1 CNC machine but is designed to be compatible with any GRBL-based system.

## Overview

The MR-1 OpenPendant is a hardware project that provides a modern, user-friendly interface for controlling CNC machines running GRBL firmware. This repository contains the firmware implementation that powers the pendant device.

The hardware (PCB) design files are available in the main project repository: [MR-1 OpenPendant Hardware](https://github.com/cumhuronat/mr-1-openpendant/tree/main/pendant)

## Features

- **Bluetooth Connectivity**: Implements BLE (Bluetooth Low Energy) communication with GRBL controllers
- **Real-time Machine Status**: Monitors and displays:
  - Work position (X, Y, Z coordinates)
  - Feed rate and spindle speed
  - Machine status
  - Pin states
  - Work coordinate offsets
  - Override values
- Modern UI with LVGL graphics library
- **Extensible Design**: Open-source architecture allowing for community contributions and customizations

## Development Requirements

- ESP-IDF development framework
- CMake build system

## Building the Firmware

1. Install ESP-IDF and set up the development environment
2. Clone this repository:
   ```bash
   git clone [repository-url]
   ```
3. Initialize submodules:
   ```bash
   git submodule update --init --recursive
   ```
4. Build the project:
   ```bash
   idf.py build
   ```
5. Flash to your device:
   ```bash
   idf.py -p [PORT] flash
   ```

## Project Structure

```
pendant-software/
├── components/          # Project components
│   ├── ble_manager/    # Bluetooth communication
│   ├── drivers/        # Hardware drivers
│   └── ui_manager/     # User interface
├── main/               # Main application code
├── CMakeLists.txt     # Build configuration
└── sdkconfig          # ESP-IDF configuration
```

## Contributing

We welcome contributions to the MR-1 OpenPendant project! Here's how you can help:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

Please ensure your code:
- Follows the existing coding style
- Includes appropriate documentation
- Has been tested with target hardware


## Contact & Support

For questions, suggestions, or support:
- Open an issue in this repository
- Visit the main project repository: [MR-1 OpenPendant](https://github.com/cumhuronat/mr-1-openpendant)