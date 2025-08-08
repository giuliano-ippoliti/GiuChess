# 🏰 GiuChess 2.0 - Release Notes

## Version 2.0.0 - Major Code Cleanup & UCI Support

**Release Date:** August 8, 2025  
**Build:** Professional, zero-warning compilation

---

## 🎉 What's New

### ✨ Major Improvements
- **🧹 Complete code cleanup**: Removed 60+ unused variables across all source files
- **🔧 Zero compilation warnings**: Professional codebase with `-Wall` clean compilation
- **🆕 UCI protocol support**: Full compatibility with modern chess GUIs
- **📚 Comprehensive documentation**: Professional README and development guides
- **🛡️ Enhanced robustness**: Added missing function prototypes and improved code quality

### 🎮 Chess Engine Features
- **Dual Protocol Support**: Both UCI and XBoard protocols
- **Arena GUI Compatible**: Seamless integration with Arena Chess GUI
- **Configurable Engine**: Adjustable search depth (1-10), hash size (1-1024 MB)
- **Professional Quality**: Clean, maintainable codebase ready for contributions

---

## 📦 Downloads

### Windows
- **GiuChess-2.0.exe** - Ready-to-use Windows executable
- Compatible with Windows 7/8/10/11 (64-bit)

### Source Code
- Full C source code included for compilation on other platforms
- Makefile provided for Linux/Unix systems

---

## 🚀 Quick Start

### For Chess Players
1. Download `GiuChess-2.0.exe`
2. Install [Arena Chess GUI](http://www.playwitharena.de/) or any UCI-compatible interface
3. Add GiuChess as a new UCI engine
4. Start playing!

### For Developers
1. Clone the repository
2. Run `make` (Linux/Unix) or use the provided compilation commands
3. See `README.md` for detailed build instructions

---

## 🔧 Technical Details

### Compilation
- **Compiler**: GCC with `-Wall -O2` optimization
- **Dependencies**: Standard C libraries, math library (`-lm`)
- **Size**: ~636 KB executable
- **Warnings**: Zero compilation warnings ✅

### Engine Specifications
- **Search Algorithm**: Alpha-beta pruning
- **Evaluation**: Material and positional evaluation
- **Move Generation**: Bitboard-based legal move generation
- **Protocols**: UCI and XBoard/WinBoard

---

## 📈 Version History

### Version 2.0.0 (2025)
- Major code cleanup and modernization
- UCI protocol support added
- Professional documentation
- Zero compilation warnings
- Enhanced robustness and reliability

### Version 1.0 (2005)
- Original release by Giuliano Ippoliti
- XBoard protocol support
- Basic chess engine functionality

---

## 🙏 Credits

- **Original Author**: Giuliano Ippoliti (2005)
- **Code Cleanup & UCI Support**: Enhanced with Claude 4 assistance (2025)
- **License**: GNU General Public License v2.0

---

## 🔗 Links

- [Repository](https://github.com/giuliano-ippoliti/GiuChess)
- [UCI Protocol Documentation](README_UCI.md)
- [Development Documentation](CODE_CLEANUP.md)
- [Arena Chess GUI](http://www.playwitharena.de/)

---

*GiuChess 2.0 - A professional chess engine with modern compatibility* 🏆
