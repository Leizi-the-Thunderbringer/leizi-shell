# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Leizi Shell** is a modern POSIX-compatible shell written in C++20, featuring ZSH-style arrays and Powerlevel10k-inspired prompts. Currently version **1.4.0** (2025-10-04).

## 🎉 Latest Release: v1.4.0 - Package Management & Distribution (2025-10-04)

### Major Achievement
- **提前 6 个月 27 天完成** (超出预期 239%)
- 原计划: 2026年5月31日
- 实际发布: 2025年10月4日

### New Features in v1.4.0
1. **📦 Multi-Platform Package Management**
   - **Homebrew (macOS)**: `brew install Zixiao-System/leizi/leizi`
     - Tap repository: https://github.com/Zixiao-System/homebrew-leizi
     - Formula: `Formula/leizi.rb`
     - SHA256: `8241ed63466189da7a1f6269500bf88b67c48661ed04236cb7d4646188a3e314`
   - **Docker**: Multi-arch support (amd64/arm64)
     - `docker pull leizi/leizi-shell:1.4.0`
     - Dockerfile: `packaging/docker/Dockerfile`
   - **Debian/Ubuntu**: APT package support
     - Files: `packaging/debian/` (control, changelog, rules, etc.)
   - **Arch Linux**: AUR package
     - PKGBUILD: `packaging/arch/PKGBUILD`

2. **🤖 CI/CD Automation**
   - GitHub Actions: `.github/workflows/package-release.yml`
   - Automated release workflow
   - Multi-platform parallel builds
   - Automatic artifact uploads

## Recent Major Updates

### v1.3.0 (2025-10-03) - User Experience & Documentation
1. **Syntax Highlighting** ✅
   - Fish Shell style real-time syntax highlighting
   - Command validation (PATH + builtin)
   - Variable recognition ($VAR, ${VAR})
   - String detection, operator highlighting
   - Files: `src/syntax/highlighter.h/.cpp`, `src/builtin/highlight.cpp`

2. **Performance Optimization** ✅
   - Startup time: 2ms (超出目标 25倍)
   - Prompt generation: 2ms (超出目标 50倍)
   - Git status caching (10s branch + 2s status)
   - Scripts: `scripts/benchmark.sh`, `scripts/valgrind-check.sh`

3. **Configuration System** ✅
   - INI-style config: `~/.config/leizi/config`
   - Files: `src/config/config.h/.cpp`
   - Sections: [prompt], [completion], [history], [aliases]

4. **Smart Completion** ✅
   - Provider pattern architecture
   - Files: `src/completion/completer.h/.cpp`
   - Command, variable, history, file completion

5. **Testing Suite** ✅
   - Catch2 framework (v2.13.10)
   - Unit tests: `tests/unit/`
   - Integration tests: `tests/integration/`
   - 100% pass rate (2/2 tests)

6. **Complete Documentation** ✅
   - User guide: `docs/user-guide/README.md`
   - API docs: `docs/api/README.md`
   - Doxygen guide: `docs/DOXYGEN.md`

### v1.2.0 - Modularization & Testing
1. **Code Modularization** ✅
   - Separated monolithic `main.cpp` into modules
   - Structure:
     ```
     src/
     ├── core/           # Parser, job control
     ├── builtin/        # Individual command implementations
     ├── prompt/         # Prompt generation, Git integration
     ├── completion/     # Tab completion
     ├── config/         # Configuration management
     ├── syntax/         # Syntax highlighting
     └── utils/          # Colors, variables, signal handling
     ```

2. **Built-in Commands Modularization** ✅
   - Files: `src/builtin/*.cpp`
   - Manager: `src/builtin/builtin_manager.h/.cpp`

### v1.1.1 (2025-09-26) - Job Control
1. **Job Control System** ✅
   - Background execution with `&` operator
   - Commands: `jobs`, `fg`, `bg`
   - Ctrl+Z suspension support
   - Files: `src/core/job_control.h/.cpp`

### v1.1.0 (2025-09-25) - Pipes & I/O
1. **Pipe Support** ✅
   - Full command piping with `|` operator
   - Multi-level pipes support
   - Files: `src/core/parser.cpp`

2. **I/O Redirection** ✅
   - Operators: `>`, `>>`, `<`, `2>`, `2>>`, `&>`
   - Works with builtin and external commands

3. **Project Rename** ✅
   - Renamed from "Lezi" to "Leizi"

## Build Commands

### Build for Development
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Build for Release
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Build with Sanitizers
```bash
# AddressSanitizer + UBSan
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON ..
make -j$(nproc)

# ThreadSanitizer
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_TSAN=ON ..
make -j$(nproc)

# Profiling
cmake -DENABLE_PROFILING=ON ..
make -j$(nproc)
```

### Run Tests
```bash
# Unit tests
cd build && ctest --output-on-failure

# Benchmarks
./scripts/benchmark.sh

# Memory checks
./scripts/valgrind-check.sh
```

### Install
```bash
# From source
sudo make install

# Homebrew (macOS)
brew install Zixiao-System/leizi/leizi

# Docker
docker pull leizi/leizi-shell:latest
```

## Architecture

### Current Modular Structure ✅
```
src/
├── core/
│   ├── parser.h/.cpp           # Command parsing
│   └── job_control.h/.cpp      # Job management
├── builtin/
│   ├── builtin.h               # Base class
│   ├── builtin_manager.h/.cpp  # Command registry
│   ├── cd.cpp                  # cd command
│   ├── echo.cpp                # echo command
│   ├── export.cpp              # export/unset
│   ├── array.cpp               # array command
│   ├── history.cpp             # history command
│   ├── simple.cpp              # pwd/exit/clear
│   ├── info.cpp                # help/version
│   └── highlight.cpp           # highlight demo
├── prompt/
│   ├── prompt.h/.cpp           # Prompt generator
│   └── git.h/.cpp              # Git integration
├── completion/
│   └── completer.h/.cpp        # Tab completion
├── config/
│   └── config.h/.cpp           # Configuration system
├── syntax/
│   └── highlighter.h/.cpp      # Syntax highlighting
├── utils/
│   ├── colors.h                # Color constants
│   ├── variables.h/.cpp        # Variable manager
│   └── signal_handler.h/.cpp   # Signal handling
└── main.cpp                    # Main entry point
```

### Package Management Structure (NEW in v1.4.0)
```
packaging/
├── homebrew/
│   └── leizi.rb                # Homebrew Formula
├── debian/
│   ├── control                 # Package metadata
│   ├── changelog               # Version history
│   ├── copyright               # License info
│   ├── rules                   # Build rules
│   ├── compat                  # debhelper version
│   └── install                 # Install list
├── arch/
│   └── PKGBUILD                # Arch package build
└── docker/
    ├── Dockerfile              # Multi-stage build
    └── README.md               # Docker usage
```

## Key Implementation Details

### Variable System
- Location: `src/utils/variables.h/.cpp`
- Uses `std::variant` for type safety
- Special variables: `$?`, `$$`, `$PWD`, `$HOME`, `$USER`
- Variable expansion: `$var` and `${var}` syntax
- Arrays: ZSH-style syntax `array name=(value1 value2)`

### Built-in Commands
- **Core**: `cd`, `pwd`, `exit`, `clear` (`src/builtin/simple.cpp`)
- **Variables**: `export`, `unset`, `env` (`src/builtin/export.cpp`)
- **Arrays**: `array` (`src/builtin/array.cpp`)
- **History**: `history` (`src/builtin/history.cpp`)
- **Info**: `help`, `version` (`src/builtin/info.cpp`)
- **Job Control**: `jobs`, `fg`, `bg` (`src/core/job_control.cpp`)
- **Demo**: `highlight` (`src/builtin/highlight.cpp`)

### Prompt System
- Location: `src/prompt/prompt.cpp`
- Powerlevel10k-inspired design
- Git integration: `src/prompt/git.cpp`
- Format: `user@hostname ~/path (git-branch) ✓ [exit-code] timestamp`
- Smart caching (10s branch, 2s status)

### Parser System
- Location: `src/core/parser.cpp`
- Tokenization with special operator handling
- Pipe parsing: `|`
- Redirection: `>`, `>>`, `<`, `2>`, `2>>`, `&>`
- Background jobs: `&`

### Job Control
- Location: `src/core/job_control.cpp`
- Process group management
- Job status tracking (Running, Stopped, Done)
- Signal handling (SIGTSTP, SIGCHLD)

## Development Notes

### Coding Standards
- **C++ Standard**: C++20 with modern features
- **Memory Safety**: RAII pattern, smart pointers only
- **Error Handling**: Exceptions preferred
- **Naming**:
  - Classes: `PascalCase`
  - Functions: `camelCase`
  - Variables: `camelCase`
  - Constants: `UPPER_CASE`

### Dependencies
- **Required**: C++20 compiler (GCC 10+, Clang 12+), CMake 3.16+
- **Optional**: libreadline (enhanced line editing)

### Platform Support
- Linux (primary)
- macOS (Homebrew configured)
- BSD systems
- Docker (multi-arch)

## Current Development Status

### Completed (v1.0 - v1.4.0)
- ✅ TASK-001: Project rename to "Leizi"
- ✅ TASK-003: Code modularization
- ✅ TASK-005: Pipe support
- ✅ TASK-006: I/O redirection
- ✅ TASK-007: Job control system
- ✅ TASK-008: Enhanced auto-completion
- ✅ TASK-009: Configuration system
- ✅ TASK-010: Syntax highlighting
- ✅ TASK-011: Testing suite (Catch2)
- ✅ TASK-012: Memory safety & performance
- ✅ TASK-013: Complete documentation
- ✅ TASK-014: Package management (Homebrew, Docker, Debian, Arch)

### Next Steps (v2.0.0-preview)
See TODO.md and ROADMAP.md for details:
- TASK-015: Shell scripting support (functions, if/case, loops)
- TASK-016: Plugin system architecture
- TASK-017: Network/cloud integration

## Testing

### Test Structure
```
tests/
├── catch.hpp                   # Catch2 v2.13.10
├── unit/                       # Unit tests
│   ├── test_parser.cpp
│   ├── test_variables.cpp
│   └── test_builtin.cpp
└── integration/                # Integration tests
    ├── test_main.cpp
    ├── test_pipeline.cpp
    ├── test_redirection.cpp
    └── test_jobs.cpp
```

### Running Tests
```bash
# CMake tests
cd build && ctest --output-on-failure

# Performance benchmarks
./scripts/benchmark.sh

# Memory leak detection
./scripts/valgrind-check.sh
```

## Package Management (NEW in v1.4.0)

### Homebrew Tap
```bash
# Install
brew install Zixiao-System/leizi/leizi

# Update Formula (for maintainers)
cd /opt/homebrew/Library/Taps/zixiao-system/homebrew-leizi
# Edit Formula/leizi.rb
git commit -am "leizi: update to x.x.x"
git push
```

### Docker
```bash
# Build
docker build -t leizi-shell -f packaging/docker/Dockerfile .

# Run
docker run -it leizi-shell

# Multi-arch build
docker buildx build --platform linux/amd64,linux/arm64 \
  -t leizi/leizi-shell:1.4.0 --push .
```

### Debian Package
```bash
# Build
cd leizi-shell
dpkg-buildpackage -us -uc -b

# Install
sudo dpkg -i ../leizi-shell_1.4.0_amd64.deb
```

### Arch Package
```bash
# Build
cd packaging/arch
makepkg -si

# Update PKGBUILD version
sed -i 's/pkgver=.*/pkgver=x.x.x/' PKGBUILD
```

## Git Workflow
- Main branch: `main`
- Feature branches: `feature/*`
- CI/CD: GitHub Actions
- **GPG signing required**: `git commit -S`
- **Co-authorship**: `Co-Authored-By: HwlloChen <140884226+HwlloChen@users.noreply.github.com>`

## Version History
- v1.4.0 (2025-10-04): Package Management & Distribution
- v1.3.0 (2025-10-03): Syntax Highlighting & Performance
- v1.2.0: Modularization & Testing
- v1.1.1 (2025-09-26): Job Control System
- v1.1.0 (2025-09-25): Pipes & I/O Redirection
- v1.0.1 (2025-09-16): Bug fixes
- v1.0.0 (2025-09-15): Initial release

## Important Implementation Notes

### Pipe Implementation
- File: `src/core/parser.cpp::executePipeline()`
- Creates pipes with `pipe()`
- Manages file descriptors with `dup2()`
- Proper synchronization with `waitpid()`

### I/O Redirection
- File: `src/core/parser.cpp::parseRedirection()`
- Tokenizes operators separately
- Tracks redirection type and filename
- Manages file descriptors safely

### Job Control
- File: `src/core/job_control.cpp`
- Process groups with `setpgid()`
- Signal handling (SIGCHLD, SIGTSTP)
- Job status tracking and cleanup

### Syntax Highlighting
- File: `src/syntax/highlighter.cpp`
- Command validation (PATH + builtin)
- Variable recognition
- PATH command caching

### Configuration
- File: `src/config/config.cpp`
- INI parser with section support
- Type inference (bool, int, string)
- Alias expansion

## Testing Commands
```bash
# Pipes
echo "hello world" | grep hello
ls -la | grep main | wc -l

# Redirection
echo "test" > file.txt
echo "append" >> file.txt
cat < file.txt
ls nonexistent 2> error.txt

# Job control
sleep 30 &           # Background
jobs                 # List jobs
fg %1                # Foreground job 1
bg %1                # Background job 1

# Syntax highlighting
highlight echo hello world
highlight ls -la | grep test > file.txt

# Arrays
array fruits=(apple banana orange)
array fruits

# Configuration
cat ~/.config/leizi/config
```

## Common Workflows

### Adding a New Built-in Command
1. Create `src/builtin/newcmd.cpp`
2. Inherit from `BuiltinCommand`
3. Register in `src/builtin/builtin_manager.cpp`
4. Update CMakeLists.txt
5. Add tests in `tests/unit/test_builtin.cpp`

### Debugging
```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# GDB
gdb ./build/leizi

# Valgrind
valgrind --leak-check=full ./build/leizi

# Sanitizers
cmake -DENABLE_SANITIZERS=ON ..
make
./build/leizi
```

### Release Process
1. Update version in `CMakeLists.txt`
2. Update `ROADMAP.md`
3. Commit: `git commit -S -m "release: vX.X.X"`
4. Push: `git push`
5. Create release: `gh release create vX.X.X`
6. Update Homebrew Formula SHA256
7. Update Docker image tags

## Resources
- **Main Repo**: https://github.com/Zixiao-System/leizi-shell
- **Homebrew Tap**: https://github.com/Zixiao-System/homebrew-leizi
- **Documentation**: See `docs/` directory
- **Roadmap**: See `ROADMAP.md`
- **TODO**: See `TODO.md`

---

**Last Updated**: 2025-10-04
**Current Version**: 1.4.0
**Maintainers**: @Amiya167, Claude AI
**Status**: Active Development
