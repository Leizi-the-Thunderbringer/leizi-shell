# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Leizi Shell** is a modern POSIX-compatible shell written in C++20, featuring ZSH-style arrays and Powerlevel10k-inspired prompts. Currently version 1.0.1.

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

### Build with Sanitizers (Debug/Memory Safety)
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON ..
make -j$(nproc)
```

### Run Tests
```bash
# Basic functionality test script
./test.sh

# When unit tests are implemented:
make test
```

### Install
```bash
sudo make install
# Or use the install script:
./install.sh
```

## Architecture

### Current Structure (Monolithic)
The entire shell implementation is currently in a single file:
- `src/main.cpp` (~2000+ lines) - Contains all shell logic including:
  - `LeiziShell` class - Main shell controller
  - Command parser and tokenization
  - Built-in commands (cd, echo, export, array, history, etc.)
  - Variable management (environment and shell variables)
  - ZSH-style array support
  - Git integration for prompt
  - Tab completion
  - Signal handling

### Planned Modular Structure
According to TODO.md, the code will be refactored into:
- `src/core/` - Shell core, parser, executor
- `src/builtin/` - Individual built-in command implementations
- `src/prompt/` - Prompt generation and Git integration
- `src/completion/` - Tab completion system
- `src/utils/` - Colors, variables, signal handling

## Key Implementation Details

### Variable System
- Supports both scalar and array variables
- Special variables: `$?` (exit code), `$$` (PID), `$PWD`, `$HOME`, `$USER`
- Variable expansion: `$var` and `${var}` syntax
- Arrays use ZSH-style syntax: `array name=(value1 value2)`

### Built-in Commands
- **Core**: `cd`, `pwd`, `exit`, `clear`
- **Variables**: `export`, `unset`, `env`
- **Arrays**: `array` (create/display ZSH-style arrays)
- **History**: `history` with persistent storage
- **Help**: `help`, `version`
- **Process**: `exec`

### Prompt System
- Colorful Powerlevel10k-inspired prompt
- Git branch and status integration (shows branch, modified files count)
- Format: `user@hostname ~/path (git-branch) ✓ [exit-code] timestamp`
- Smart path shortening with `~` for home directory

### External Command Execution
- Uses `fork()` and `execvp()` for external commands
- Searches PATH for executables
- Proper signal handling and process management

## Development Notes

### Coding Standards
- **C++ Standard**: C++20 with modern features
- **Memory Safety**: RAII pattern, no raw pointers, smart pointers only
- **Error Handling**: Prefer exceptions over error codes
- **Naming**:
  - Classes: `PascalCase`
  - Functions: `camelCase`
  - Variables: `camelCase`
  - Constants: `UPPER_CASE`

### Dependencies
- **Required**: C++20 compiler (GCC 10+, Clang 12+), CMake 3.16+
- **Optional**: libreadline (for enhanced line editing)

### Platform Support
- Linux (primary)
- macOS (with Homebrew paths configured)
- BSD systems

## Priority Development Tasks

From TODO.md, the most critical tasks are:

1. **TASK-001**: Rename project from "lezi" to "leizi" throughout (COMPLETED)
2. **TASK-003**: Modularize the monolithic `main.cpp` into separate components
3. **TASK-005**: Implement pipe support (`|`)
4. **TASK-006**: Implement I/O redirection (`>`, `>>`, `<`, `2>`)
5. **TASK-007**: Job control (`jobs`, `fg`, `bg`, `&`)

## Testing

Currently uses `test.sh` for basic functionality testing. Tests cover:
- Basic commands (version, help, echo)
- Variable expansion
- Array creation
- Exit codes

Future testing will use:
- Unit tests with Catch2 or Google Test
- Integration tests for pipelines and job control
- BATS for functional testing

## Common Development Workflows

### Adding a New Built-in Command
1. Add command handler in `executeBuiltin()` function
2. Implement the command logic
3. Update help text in `showHelp()`
4. Add tests in `test.sh`

### Debugging
```bash
# Build with debug symbols and sanitizers
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with GDB
gdb ./leizi

# Check for memory leaks with Valgrind
valgrind --leak-check=full ./leizi
```

### Performance Profiling
```bash
# Build with profiling info
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make

# Profile with perf (Linux)
perf record ./leizi
perf report
```

## Git Workflow
- Main branch: `main`
- Development happens on feature branches
- CI/CD via GitHub Actions (builds for Linux, macOS, Windows)
- Releases are automated through GitHub Actions