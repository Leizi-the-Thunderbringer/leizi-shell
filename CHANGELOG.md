# Changelog

All notable changes to this project will be documented in this file.

## [1.1.1] - 2025-09-26

### Added
- **Job Control System**: Complete implementation of job management features
  - Background execution with `&` operator
  - `jobs` command to list all active jobs
  - `fg [job]` command to bring jobs to foreground
  - `bg [job]` command to resume stopped jobs in background
  - Ctrl+Z (SIGTSTP) support for suspending foreground processes
  - Automatic job status tracking (Running, Stopped, Done)
  - Job cleanup after completion

### Changed
- **GitHub Actions**: Updated workflow files to use custom API endpoint
  - Added `ANTHROPIC_BASE_URL` environment variable for claude.yml
  - Added `ANTHROPIC_BASE_URL` environment variable for claude-code-review.yml

### Technical Improvements
- Enhanced signal handling to support SIGTSTP
- Improved process management with foreground/background distinction
- Added global foreground PID tracking for signal routing
- Updated version to 1.1.1

## [1.1.0] - 2025-09-25

### Added
- **Pipe Support**: Full implementation of command piping with `|` operator
  - Support for multi-level pipes (e.g., `cmd1 | cmd2 | cmd3`)
  - Proper process synchronization and file descriptor management
  - Builtin commands are prevented from being used in pipes

- **I/O Redirection**: Complete support for all major redirection operators
  - `>` - Output redirection (overwrite)
  - `>>` - Output append
  - `<` - Input redirection
- `2>` - Error redirection
  - `2>>` - Error append
  - `&>` - Redirect both stdout and stderr
  - Works with both builtin and external commands

- **Improved Command Parser**: Enhanced tokenization for special operators
  - Correctly handles redirection and pipe operators
  - Maintains quote handling and escape sequences

### Changed
- **Project Renamed**: Complete rename from "Lezi" to "Leizi" throughout the codebase
  - Updated all documentation, configuration files, and source code
  - Binary is now named `leizi`

- **Modular Structure Started**: Initial modularization work
  - Created `src/utils/colors.h` for color constants
  - Created `src/utils/variables.h/.cpp` for improved variable management with `std::variant`
  - Directory structure prepared for future modularization

### Technical Improvements
- Enhanced command execution flow to support complex pipelines
- Improved error handling for file operations
- Better process management with proper signal handling

## [1.0.1] - 2025-09-16

### Fixed
- showVersion function to correctly display version information and repository URL
- Minor bug fixes and performance improvements

## [1.0.0] - 2025-09-15

### Added
- Initial release of Lezi Shell
- Beautiful Powerlevel10k-inspired prompts
- Git integration with branch and status display
- ZSH-style array support
- Smart tab completion (when readline available)
- POSIX compatibility
- Variable expansion ($var, ${var})
- Command history with persistent storage
- Cross-platform support (Linux, macOS)

### Features
- Built-in commands: cd, pwd, echo, export, unset, array, history, clear, help, version, exit
- Colorful and informative prompts
- Memory-safe C++20 implementation
