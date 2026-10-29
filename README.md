# 🏰 GiuChess 2.0

A powerful chess engine written in C, supporting both XBoard and UCI protocols.

## 📋 Overview

GiuChess is a chess engine originally developed by Giuliano Ippoliti in 2005. This version 2.0 has been significantly improved with:

- **Zero compilation warnings** - Professional, clean codebase
- **UCI protocol support** - Compatible with modern chess GUIs
- **XBoard compatibility** - Works with traditional interfaces
- **Optimized code** - Removed unused variables and improved structure

## ✨ Features

- **Dual Protocol Support**: Both UCI and XBoard protocols
- **Arena GUI Compatible**: Works seamlessly with Arena Chess GUI
- **Configurable Depth**: Adjustable search depth (1-10 levels)
- **Hash Table Support**: Configurable hash size (1-1024 MB)
- **Multi-threading Ready**: Thread configuration support
- **Professional Codebase**: Zero compilation warnings with `-Wall`

## 🚀 Quick Start

### Compilation

```bash
# Linux/Unix
make

# Windows (with GCC)
gcc -Wall -g -O2 -o GiuChess-2.0.exe *.c -lm
```

### Running the Engine

#### UCI Mode (Recommended)
```bash
./GiuChess-2.0 uci
```

#### XBoard Mode
```bash
./GiuChess-2.0
```

## 🎮 Using with Chess GUIs

### Arena Chess GUI
1. Download and install [Arena Chess GUI](http://www.playwitharena.de/)
2. Go to `Engines` → `Install New Engine`
3. Browse to your compiled `GiuChess-2.0.exe`
4. Select "UCI" as the protocol
5. Click OK and start playing!

### Other UCI-Compatible GUIs
GiuChess 2.0 works with any UCI-compatible chess interface:
- ChessBase
- Fritz
- Scid vs. PC
- Cute Chess
- And many more...

## ⚙️ Configuration

### UCI Options
- **Hash**: Memory for hash tables (1-1024 MB, default: 32)
- **Threads**: Number of threads (1-1, default: 1)
- **Depth**: Search depth (1-10, default: 5)

### Example UCI Session
```
uci
setoption name Hash value 64
setoption name Depth value 6
isready
position startpos moves e2e4 e7e5
go depth 5
```

## 🛠️ Development

### Project Structure
```
GiuChess/
├── *.c, *.h          # Core engine source files
├── uci.c, uci.h      # UCI protocol implementation
├── Makefile          # Build configuration
├── scripts/          # Utility scripts
├── CODE_CLEANUP.md   # Development documentation
└── README_UCI.md     # UCI-specific documentation
```

### Building from Source

#### Prerequisites
- GCC compiler
- Make utility
- Standard C libraries

#### Compilation Options
```bash
# Debug build
make debug

# Release build (optimized)
make

# Clean build files
make clean
```

## 📚 Documentation

- [`README_UCI.md`](README_UCI.md) - UCI protocol usage guide
- [`CODE_CLEANUP.md`](CODE_CLEANUP.md) - Development and cleanup documentation
- [`test_uci.txt`](test_uci.txt) - UCI testing commands

## 🧪 Testing

### Perft (move generation check)
```bash
make perft
```

Builds `perft-test` (from [`perft.c`](perft.c) and the engine sources) and counts the leaf nodes of the legal move tree, comparing them to reference values from [chessprogramming.org](https://www.chessprogramming.org/Perft_Results):

- Starting position, depths 1-5: 20, 400, 8902, 197281, 4865609
- Kiwipete (`r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -`), depths 1-3: 48, 2039, 97862
- Position 3 (`8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -`, en passant and pins), depths 1-5: 14, 191, 2812, 43238, 674624

Each line prints `OK` or `MISMATCH`; the command exits with a non-zero status if any count differs.

### Quick UCI Test
```bash
echo -e "uci\nisready\nquit" | ./GiuChess-2.0 uci
```

Expected output:
```
id name GiuChess-2.0
id author Giuliano Ippoliti
option name Hash type spin default 32 min 1 max 1024
option name Threads type spin default 1 min 1 max 1
option name Depth type spin default 5 min 1 max 10
uciok
readyok
```

### XBoard Test
Use the provided scripts in the `scripts/` directory for XBoard testing.

## 📈 Version History

### Version 2.0 (2025)
- ✅ **Major code cleanup**: Removed 60+ unused variables
- ✅ **Zero compilation warnings**: Professional codebase with `-Wall`
- ✅ **UCI protocol support**: Modern chess GUI compatibility
- ✅ **Improved documentation**: Comprehensive guides and examples
- ✅ **Enhanced build system**: Streamlined compilation process

### Version 1.0 (2005)
- Initial release by Giuliano Ippoliti
- XBoard protocol support
- Basic chess engine functionality

## 👥 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes with proper testing
4. Ensure zero compilation warnings
5. Submit a pull request

## 📄 License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

## 👨‍💻 Authors

- **Giuliano Ippoliti** - Original author (2005)
- **Code cleanup and UCI support** - Enhanced with Claude 4 assistance (2025)

## 🔗 Links

- [GNU General Public License](https://www.gnu.org/licenses/gpl-2.0.html)
- [UCI Protocol Specification](http://wbec-ridderkerk.nl/html/UCIProtocol.html)
- [XBoard Protocol](https://www.gnu.org/software/xboard/engine-intf.html)

---

*GiuChess 2.0 - A professional chess engine with modern compatibility* 🏆
