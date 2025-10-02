#!/bin/bash

# Leizi Shell Installation Script
# Usage: curl -sSL https://raw.githubusercontent.com/Zixiao-Tech/leizi-shell/main/install.sh | bash

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Configuration
REPO_URL="https://github.com/Zixiao-Tech/leizi-shell"
BINARY_NAME="leizi"
INSTALL_DIR="/usr/local/bin"
VERSION="latest"

# Functions
log() {
    echo -e "${CYAN}[INFO]${NC} $1"
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

fatal() {
    error "$1"
    exit 1
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Detect OS and architecture
detect_platform() {
    local os arch

    # Detect OS
    case "$(uname -s)" in
        Linux*)  os="linux";;
        Darwin*) os="darwin";;
        CYGWIN*) os="windows";;
        MINGW*)  os="windows";;
        *)       fatal "Unsupported operating system: $(uname -s)";;
    esac

    # Detect architecture
    case "$(uname -m)" in
        x86_64|amd64) arch="x64";;
        arm64|aarch64) arch="arm64";;
        armv7l) arch="arm";;
        i386|i686) arch="x86";;
        *) fatal "Unsupported architecture: $(uname -m)";;
    esac

    echo "${os}_${arch}"
}

# Check system dependencies
check_dependencies() {
    log "Checking system dependencies..."

    # Required tools
    local required_tools=("curl" "tar")
    local missing_tools=()

    for tool in "${required_tools[@]}"; do
        if ! command_exists "$tool"; then
            missing_tools+=("$tool")
        fi
    done

    if [ ${#missing_tools[@]} -ne 0 ]; then
        error "Missing required tools: ${missing_tools[*]}"
        echo
        echo "Please install missing dependencies:"
        echo "  Ubuntu/Debian: sudo apt-get install ${missing_tools[*]}"
        echo "  CentOS/RHEL:   sudo yum install ${missing_tools[*]}"
        echo "  macOS:         brew install ${missing_tools[*]}"
        exit 1
    fi

    # Optional tools for building from source
    local build_tools=("gcc" "g++" "cmake" "make")
    local missing_build_tools=()

    for tool in "${build_tools[@]}"; do
        if ! command_exists "$tool"; then
            missing_build_tools+=("$tool")
        fi
    done

    if [ ${#missing_build_tools[@]} -ne 0 ]; then
        warning "Build tools not found: ${missing_build_tools[*]}"
        warning "Pre-built binaries will be used if available"
        return 1
    fi

    return 0
}

# Install build dependencies
install_build_deps() {
    log "Installing build dependencies..."

    if command_exists apt-get; then
        # Ubuntu/Debian
        sudo apt-get update
        sudo apt-get install -y build-essential cmake libreadline-dev git
    elif command_exists yum; then
        # CentOS/RHEL (older)
        sudo yum groupinstall -y "Development Tools"
        sudo yum install -y cmake readline-devel git
    elif command_exists dnf; then
        # Fedora/RHEL (newer)
        sudo dnf groupinstall -y "Development Tools"
        sudo dnf install -y cmake readline-devel git
    elif command_exists brew; then
        # macOS
        brew install cmake readline git
    else
        fatal "Cannot install build dependencies automatically. Please install manually."
    fi
}

# Download and install pre-built binary
install_prebuilt() {
    local platform="$1"
    local download_url="${REPO_URL}/releases/latest/download/leizi-${platform}.tar.gz"
    local temp_dir

    log "Attempting to download pre-built binary for ${platform}..."

    temp_dir=$(mktemp -d)
    trap "rm -rf $temp_dir" EXIT

    # Download binary
    if ! curl -fsSL "$download_url" -o "$temp_dir/leizi.tar.gz"; then
        warning "Pre-built binary not available for ${platform}"
        return 1
    fi

    # Extract binary
    cd "$temp_dir"
    tar -xzf leizi.tar.gz

    # Install binary
    if [ -w "$INSTALL_DIR" ]; then
        cp leizi "$INSTALL_DIR/"
    else
        sudo cp leizi "$INSTALL_DIR/"
    fi

    # Make executable
    if [ -w "$INSTALL_DIR/leizi" ]; then
        chmod +x "$INSTALL_DIR/leizi"
    else
        sudo chmod +x "$INSTALL_DIR/leizi"
    fi

    success "Pre-built binary installed successfully!"
    return 0
}

# Build and install from source
install_from_source() {
    local temp_dir
    local build_deps_installed=false

    log "Building Leizi Shell from source..."

    # Check if we need to install build dependencies
    if ! check_dependencies >/dev/null 2>&1; then
        read -p "Install build dependencies? [Y/n] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Nn]$ ]]; then
            fatal "Build dependencies required for source installation"
        fi
        install_build_deps
        build_deps_installed=true
    fi

    temp_dir=$(mktemp -d)
    trap "rm -rf $temp_dir" EXIT

    # Clone repository
    log "Cloning repository..."
    cd "$temp_dir"
    git clone "$REPO_URL" leizi-shell
    cd leizi-shell

    # Get latest release if requested
    if [ "$VERSION" = "latest" ]; then
        local latest_tag
        latest_tag=$(git describe --tags --abbrev=0 2>/dev/null || echo "main")
        git checkout "$latest_tag"
        log "Building version: $latest_tag"
    else
        git checkout "$VERSION"
        log "Building version: $VERSION"
    fi

    # Build
    log "Configuring build..."
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..

    log "Building (this may take a few minutes)..."
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

    log "Installing..."
    if [ -w "/usr/local/bin" ]; then
        make install
    else
        sudo make install
    fi

    success "Built and installed from source successfully!"
}

# Add shell to /etc/shells
add_to_shells() {
    local shell_path="$INSTALL_DIR/$BINARY_NAME"

    if [ ! -f "/etc/shells" ]; then
        warning "/etc/shells not found, skipping shell registration"
        return
    fi

    if grep -q "$shell_path" /etc/shells; then
        log "Leizi Shell already registered in /etc/shells"
        return
    fi

    log "Adding Leizi Shell to /etc/shells..."
    if [ -w "/etc/shells" ]; then
        echo "$shell_path" >> /etc/shells
    else
        echo "$shell_path" | sudo tee -a /etc/shells >/dev/null
    fi

    success "Added to /etc/shells"
}

# Set as default shell
set_default_shell() {
    local shell_path="$INSTALL_DIR/$BINARY_NAME"

    echo
    read -p "Set Leizi Shell as your default shell? [y/N] " -n 1 -r
    echo

    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        return
    fi

    if ! grep -q "$shell_path" /etc/shells; then
        add_to_shells
    fi

    log "Setting Leizi Shell as default shell..."
    chsh -s "$shell_path"
    success "Default shell changed! Please log out and back in for changes to take effect."
}

# Show installation summary
show_summary() {
    local shell_path="$INSTALL_DIR/$BINARY_NAME"

    echo
    echo -e "${BOLD}${GREEN}🎉 Leizi Shell Installation Complete!${NC}"
    echo
    echo "Installation location: $shell_path"
    echo "Version: $($shell_path --version 2>/dev/null || echo 'unknown')"
    echo
    echo -e "${BOLD}Next steps:${NC}"
    echo "1. Run 'leizi' to start the shell"
    echo "2. Type 'help' for available commands"
    echo "3. Visit $REPO_URL for documentation"
    echo
    echo -e "${BOLD}Features available:${NC}"
    echo "  ✅ Beautiful Powerlevel10k-inspired prompts"
    echo "  ✅ Git integration"
    echo "  ✅ ZSH-style arrays"
    echo "  ✅ Smart tab completion"
    echo "  ✅ POSIX compatibility"
    echo "  ✅ Command history"
    echo
}

# Main installation function
main() {
    echo -e "${BOLD}${CYAN}🚀 Leizi Shell Installer${NC}"
    echo -e "Modern POSIX-compatible shell with beautiful prompts"
    echo

    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --version)
                VERSION="$2"
                shift 2
                ;;
            --install-dir)
                INSTALL_DIR="$2"
                shift 2
                ;;
            --source-only)
                SOURCE_ONLY=true
                shift
                ;;
            --help)
                echo "Usage: $0 [OPTIONS]"
                echo "Options:"
                echo "  --version VERSION    Install specific version (default: latest)"
                echo "  --install-dir DIR    Install directory (default: /usr/local/bin)"
                echo "  --source-only        Build from source only"
                echo "  --help               Show this help"
                exit 0
                ;;
            *)
                fatal "Unknown option: $1"
                ;;
        esac
    done

    # Detect platform
    local platform
    platform=$(detect_platform)
    log "Detected platform: $platform"

    # Check dependencies
    check_dependencies
    local has_build_tools=$?

    # Try installation methods
    local install_success=false

    # Try pre-built binary first (unless source-only requested)
    if [ "$SOURCE_ONLY" != "true" ]; then
        if install_prebuilt "$platform"; then
            install_success=true
        fi
    fi

    # Fall back to source build
    if [ "$install_success" = "false" ]; then
        if [ $has_build_tools -eq 0 ] || [ "$SOURCE_ONLY" = "true" ]; then
            install_from_source
            install_success=true
        else
            fatal "No pre-built binary available and build tools not found"
        fi
    fi

    # Verify installation
    if ! command_exists "$BINARY_NAME"; then
        fatal "Installation failed: $BINARY_NAME command not found"
    fi

    # Post-installation steps
    add_to_shells
    set_default_shell
    show_summary
}

# Run main function
main "$@"