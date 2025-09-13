# Contributing to Lezi Shell

Thank you for your interest in contributing to Lezi Shell! We welcome contributions from everyone.

## 🚀 Getting Started

### Development Environment

1. **Clone the repository**
   ```bash
   git clone https://github.com/your-username/lezi-shell.git
   cd lezi-shell
   ```

2. **Install dependencies**

   **Ubuntu/Debian:**
   ```bash
   sudo apt-get update
   sudo apt-get install build-essential cmake libreadline-dev
   ```

   **macOS:**
   ```bash
   brew install cmake readline
   ```

   **Fedora/RHEL:**
   ```bash
   sudo dnf install gcc-c++ cmake readline-devel
   ```

3. **Build in development mode**
   ```bash
   mkdir build && cd build
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   make -j$(nproc)
   ```

### Code Style

We follow modern C++20 best practices:

- **Memory Safety**: Use RAII, smart pointers, and avoid raw pointers
- **No UB**: Zero tolerance for undefined behavior
- **Const Correctness**: Use `const` wherever possible
- **Exception Safety**: Use RAII and exception-safe code patterns
- **Naming Convention**:
    - Classes: `PascalCase`
    - Functions: `camelCase`
    - Variables: `camelCase`
    - Constants: `UPPER_CASE`
    - Private members: trailing underscore or clear naming

#### Example Code Style:

```cpp
class CommandParser {
private:
    std::vector<std::string> tokens_;
    bool isValid_ = false;
    
public:
    explicit CommandParser(const std::string& input) 
        : tokens_(parseTokens(input)), isValid_(true) {}
    
    const std::vector<std::string>& getTokens() const noexcept {
        return tokens_;
    }
    
    bool isValid() const noexcept { return isValid_; }
    
private:
    std::vector<std::string> parseTokens(const std::string& input) const;
};
```

### Testing

All contributions should include appropriate tests:

```bash
# Run tests
cd build
make test

# Or run specific tests
./tests/test_parser
./tests/test_variables
```

## 🐛 Bug Reports

When reporting bugs, please include:

1. **Environment Information**
    - OS and version
    - Lezi Shell version (`lezi --version`)
    - Compiler version
    - CMake version

2. **Steps to Reproduce**
    - Exact commands used
    - Input that caused the issue
    - Expected behavior
    - Actual behavior

3. **Debug Information**
   ```bash
   # Build debug version
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   make
   
   # Run with additional debugging (if applicable)
   ./lezi --debug
   ```

## ✨ Feature Requests

For new features:

1. **Check existing issues** to avoid duplicates
2. **Describe the feature** clearly with use cases
3. **Consider backwards compatibility**
4. **Provide implementation ideas** if possible

### Priority Features

We're particularly interested in contributions for:

- [ ] Job control (`jobs`, `fg`, `bg`)
- [ ] Piping and redirection (`|`, `>`, `<`, `>>`)
- [ ] Command substitution (`` `command` ``, `$(command)`)
- [ ] Functions and aliases
- [ ] Configuration files
- [ ] Syntax highlighting
- [ ] Auto-suggestions

## 📝 Pull Request Process

1. **Fork the repository** and create a feature branch
   ```bash
   git checkout -b feature/amazing-feature
   ```

2. **Make your changes** following the code style guidelines

3. **Add/update tests** for your changes

4. **Update documentation** if needed

5. **Commit with clear messages**
   ```bash
   git commit -m "feat: add job control support
   
   - Implement basic job management
   - Add bg, fg, jobs commands
   - Update tests and documentation
   
   Closes #123"
   ```

6. **Push and create a Pull Request**

### Commit Message Convention

We use [Conventional Commits](https://www.conventionalcommits.org/):

- `feat:` New features
- `fix:` Bug fixes
- `docs:` Documentation changes
- `style:` Code style changes (formatting, etc.)
- `refactor:` Code refactoring
- `test:` Adding/updating tests
- `perf:` Performance improvements
- `ci:` CI/CD changes

## 🏗️ Architecture Overview

```
src/
├── main.cpp           # Main shell implementation
├── parser/            # Command parsing (future)
├── builtin/           # Built-in commands (future)
├── variables/         # Variable management (future)
└── prompt/            # Prompt generation (future)

tests/
├── unit/              # Unit tests
├── integration/       # Integration tests
└── fixtures/          # Test data

docs/
├── commands.md        # Command reference
├── development.md     # Development guide
└── architecture.md    # Architecture documentation
```

### Key Components

1. **LeziShell Class**: Main shell controller
2. **Command Parser**: Tokenizes and parses user input
3. **Variable System**: Manages shell and environment variables
4. **Built-in Commands**: Internal command implementations
5. **External Commands**: Process management for external programs
6. **Prompt Generator**: Creates beautiful, informative prompts

## 🔒 Security Considerations

When contributing:

- **Input Validation**: Always validate user input
- **Memory Safety**: Use safe memory management practices
- **Path Traversal**: Be careful with file path operations
- **Command Injection**: Properly escape shell commands
- **Environment Variables**: Sanitize environment variable access

## 📖 Documentation

### Code Documentation

Use clear, concise comments:

```cpp
/**
 * Parses a command line into tokens, handling quotes and escapes
 * @param input The raw command line string
 * @return Vector of parsed tokens
 * @throws ParseError if the input is malformed
 */
std::vector<std::string> parseCommand(const std::string& input) const;
```

### User Documentation

Update relevant documentation in the `docs/` directory:

- Command reference for new built-ins
- Configuration guide for new options
- Examples for new features

## 🤝 Code Review Process

All submissions require code review:

1. **Automated Checks**: CI must pass
2. **Code Review**: At least one maintainer approval
3. **Testing**: All tests must pass
4. **Documentation**: Documentation must be updated

### Review Criteria

- Code quality and style
- Test coverage
- Performance impact
- Security implications
- Backwards compatibility
- Documentation completeness

## 📊 Performance Guidelines

- **Startup Time**: Keep shell startup fast (<100ms)
- **Memory Usage**: Minimize memory footprint
- **Command Execution**: Don't block on long-running operations
- **Prompt Generation**: Keep prompt updates fast (<10ms)

### Profiling

```bash
# Build with profiling
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_PROFILING=ON ..

# Profile with perf (Linux)
perf record ./lezi
perf report

# Profile with Instruments (macOS)
# Use Xcode Instruments
```

## 🙋‍♀️ Getting Help

- **Discussions**: Use GitHub Discussions for questions
- **Chat**: Join our Discord server (link in README)
- **Email**: Contact maintainers directly for security issues

## 📜 Code of Conduct

This project follows the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md). Please be respectful and inclusive in all interactions.

## 🎖️ Recognition

Contributors will be recognized:

- In the README contributors section
- In release notes for significant contributions
- Special recognition for major features

Thank you for contributing to Lezi Shell! 🚀