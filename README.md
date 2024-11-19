# Ping Advanced 2

A modern C++ wrapper around iputils ping with advanced statistics.

## Requirements
- C++17 compiler
- CMake 3.10+
- Meson & Dependencies (for iputils)

## Project Structure
```
ping_adv_2/
├── external/
│   └── iputils/
│       └── builddir/
├── src/
└── build/
```

## Setup (Ubuntu/Debian)
Run the setup script to install dependencies:
```bash
cd external/iputils/ci
chmod +x debian.sh
sudo ./ubuntu.sh
cd ../../..
```

## Building

1. Build iputils (from project root):
```bash
# Build location: ping_adv_2/external/iputils/builddir/
cd external/iputils
./configure && meson build
cd builddir && meson install
```

2. Build our project (from project root):
```bash
# Build location: ping_adv_2/
mkdir -p build && cd build
cmake ..
make
sudo cmake --install .
```

## Running
```bash
sudo ping_adv <target_ip> <count> <interval_ms>

Arguments:
  target_ip    - IP address to ping
  count        - Number of pings (2-50)
  interval_ms  - Time between pings (0.1-10ms)
```

## Example
```bash
sudo ping_adv 8.8.8.8 10 1.0
```
