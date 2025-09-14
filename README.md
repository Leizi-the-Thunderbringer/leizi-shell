# 🚀 Lezi Shell

A modern POSIX-compatible shell with ZSH-style arrays and beautiful Powerlevel10k-inspired prompts.

## ✨ Features

- **🎨 Beautiful Prompts**: Powerlevel10k-inspired colorful prompts with git integration
- **📊 Git Integration**: Real-time branch and status display
- **🔗 ZSH-Style Arrays**: Full support for ZSH-compatible array operations
- **⚡ POSIX Compatibility**: Works with existing shell scripts
- **🔍 Smart Completion**: Intelligent tab completion for commands and files
- **📚 Command History**: Persistent history with search
- **🎯 Variable Expansion**: Full support for `$var` and `${var}` syntax
- **🛡️ Memory Safety**: Written in modern C++20 with no undefined behavior
- **🌍 Cross-Platform**: Works on Linux, macOS, and BSD systems

## 🛠️ Installation

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 12+)
- CMake 3.16+
- Optional: libreadline for enhanced line editing

### Build from Source

```bash
# Clone the repository
git clone https://github.com/Leizi-the-Thunderbringer/lezi-shell.git
cd lezi-shell

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# Install (optional)
sudo make install
```

### Quick Install Script

```bash
curl -sSL https://raw.githubusercontent.com/Leizi-the-Thunderbringer/lezi-shell/main/install.sh | bash
```

## 🚀 Usage

### Starting Lezi Shell

```bash
# Run directly
./lezi

# Or if installed system-wide
lezi
```

### Basic Commands

```bash
# Navigate directories
cd /path/to/directory
pwd

# Environment variables
export MY_VAR="hello world"
echo $MY_VAR

# ZSH-style arrays
array fruits=(apple banana orange)
array fruits  # Display array contents

# Command history
history 10    # Show last 10 commands

# Get help
help
version
```

### Advanced Features

#### Variable Expansion
```bash
export NAME="World"
echo "Hello, $NAME!"           # Hello, World!
echo "Hello, ${NAME}!"         # Hello, World!
echo "Exit code: $?"           # Last command exit code
echo "Process ID: $$"          # Current shell PID
```

#### ZSH Arrays
```bash
# Create arrays
array colors=(red green blue)
array numbers=(1 2 3 4 5)

# Display arrays
array colors    # colors=("red" "green" "blue")

# Use in commands
echo "First color: $colors"  # Expands to first element
```

#### Git Integration
The prompt automatically detects git repositories and displays:
- Current branch name
- Working directory status (✓ for clean, symbols for changes)
- Modification indicators (●N for modified, +N for added, -N for deleted, ?N for untracked)

## 🎨 Prompt Customization

Lezi Shell features a beautiful, informative prompt that shows:

```
user@hostname ~/project (main) ✓ [0] 14:30:25
❯ 
```

- **User and hostname** in bright colors
- **Current directory** with smart path shortening
- **Git branch** in parentheses (when in a git repo)
- **Git status** with colored indicators
- **Last exit code** (only shown if non-zero)
- **Timestamp** for command tracking
- **Smart prompt symbol** (❯ for user, # for root)

## 📊 Performance

Lezi Shell is designed for performance:

- **Fast startup**: Minimal initialization overhead
- **Efficient parsing**: Optimized command parsing
- **Low memory usage**: Smart memory management with RAII
- **POSIX compliance**: Compatible with existing scripts

## 🧪 Testing

```bash
# Run built-in tests
make test

# Manual testing
./tests/run_tests.sh
```

## 📖 Documentation

- [Command Reference](docs/commands.md)
- [Configuration Guide](docs/configuration.md)
- [Development Guide](docs/development.md)
- [FAQ](docs/faq.md)

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Setup

```bash
# Clone with submodules
git clone --recursive https://github.com/Leizi-the-Thunderbringer/lezi-shell.git

# Set up development environment
cd lezi-shell
./scripts/setup-dev.sh

# Build with debug symbols
mkdir debug && cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Code Style

- Follow modern C++20 practices
- Use RAII for resource management
- Comprehensive error handling
- No undefined behavior
- Memory safety first

## 📜 License

MIT License - see [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by [Powerlevel10k](https://github.com/romkatv/powerlevel10k) for the beautiful prompts
- ZSH for array syntax inspiration
- The POSIX standard for compatibility guidelines

## 📈 Roadmap

- [ ] Job control (bg, fg, jobs)
- [ ] Piping and redirection
- [ ] Command substitution
- [ ] Functions and aliases
- [ ] Configuration file support
- [ ] Plugin system
- [ ] Syntax highlighting
- [ ] Fish-like autosuggestions

## 🐛 Bug Reports

Found a bug? Please [open an issue](https://github.com/your-username/lezi-shell/issues) with:

- Your OS and version
- Lezi Shell version (`lezi --version`)
- Steps to reproduce
- Expected vs actual behavior

## 📊 Statistics

![GitHub stars](https://img.shields.io/github/stars/Leizi-the-Thunderbringer/lezi-shell?style=social)
![GitHub forks](https://img.shields.io/github/forks/Leizi-the-Thunderbringer/lezi-shell?style=social)
![GitHub issues](https://img.shields.io/github/issues/Leizi-the-Thunderbringer/lezi-shell)
![GitHub license](https://img.shields.io/github/license/Leizi-the-Thunderbringer/lezi-shell)
![Build status](https://img.shields.io/github/workflow/status/Leizi-the-Thunderbringer/lezi-shell/CI)

---

**Made with ❤️ by the Leizi-the-Thunderbringer team**