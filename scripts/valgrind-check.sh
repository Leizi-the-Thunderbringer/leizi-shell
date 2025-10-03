#!/bin/bash
# Valgrind memory leak detection for Leizi Shell

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
LEIZI_BIN="$BUILD_DIR/leizi"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${YELLOW}=== Valgrind Memory Leak Check ===${NC}\n"

# Check if valgrind is installed
if ! command -v valgrind &> /dev/null; then
    echo -e "${RED}Error: valgrind is not installed${NC}"
    echo "Install it with:"
    echo "  macOS:  brew install valgrind"
    echo "  Ubuntu: sudo apt-get install valgrind"
    exit 1
fi

# Check if binary exists
if [ ! -f "$LEIZI_BIN" ]; then
    echo -e "${RED}Error: leizi binary not found at $LEIZI_BIN${NC}"
    echo "Please build the project first"
    exit 1
fi

# Create temporary test script
TEST_SCRIPT=$(mktemp)
cat > "$TEST_SCRIPT" << 'EOF'
version
help
echo "test"
export TEST_VAR="value"
history
exit
EOF

echo -e "${YELLOW}Running valgrind...${NC}"
echo "Test commands:"
cat "$TEST_SCRIPT"
echo ""

# Run valgrind
VALGRIND_OUTPUT=$(mktemp)
valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --log-file="$VALGRIND_OUTPUT" \
    "$LEIZI_BIN" < "$TEST_SCRIPT" > /dev/null 2>&1 || true

# Parse results
echo -e "\n${YELLOW}=== Valgrind Results ===${NC}\n"

if grep -q "ERROR SUMMARY: 0 errors" "$VALGRIND_OUTPUT"; then
    echo -e "${GREEN}✓ No memory errors detected${NC}"
else
    echo -e "${RED}✗ Memory errors found:${NC}"
    grep "ERROR SUMMARY" "$VALGRIND_OUTPUT"
fi

if grep -q "All heap blocks were freed -- no leaks are possible" "$VALGRIND_OUTPUT"; then
    echo -e "${GREEN}✓ No memory leaks detected${NC}"
    LEAK_FREE=true
elif grep -q "definitely lost: 0 bytes in 0 blocks" "$VALGRIND_OUTPUT"; then
    echo -e "${GREEN}✓ No definite memory leaks${NC}"
    LEAK_FREE=true
else
    echo -e "${RED}✗ Memory leaks detected:${NC}"
    grep "definitely lost" "$VALGRIND_OUTPUT" || true
    grep "indirectly lost" "$VALGRIND_OUTPUT" || true
    grep "possibly lost" "$VALGRIND_OUTPUT" || true
    LEAK_FREE=false
fi

echo -e "\n${YELLOW}Full report saved to: $VALGRIND_OUTPUT${NC}"
echo "View with: cat $VALGRIND_OUTPUT"

# Cleanup
rm -f "$TEST_SCRIPT"

if [ "$LEAK_FREE" = true ]; then
    echo -e "\n${GREEN}✓ Memory safety check passed!${NC}"
    exit 0
else
    echo -e "\n${RED}✗ Memory safety check failed${NC}"
    exit 1
fi
