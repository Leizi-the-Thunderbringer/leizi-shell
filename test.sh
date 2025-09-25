#!/bin/bash

# Basic test script for Leizi Shell
# Run this to verify basic functionality after building

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

BINARY="./leizi"
if [ ! -f "$BINARY" ]; then
    BINARY="./build/leizi"
fi

if [ ! -f "$BINARY" ]; then
    echo -e "${RED}Error: leizi binary not found${NC}"
    echo "Please build the project first:"
    echo "  mkdir build && cd build"
    echo "  cmake .. && make"
    exit 1
fi

echo -e "${YELLOW}Testing Leizi Shell...${NC}"
echo

# Test 1: Version command
echo -n "Testing version command... "
if echo "version" | timeout 5 "$BINARY" >/dev/null 2>&1; then
    echo -e "${GREEN}✓${NC}"
else
    echo -e "${RED}✗${NC}"
    exit 1
fi

# Test 2: Help command
echo -n "Testing help command... "
if echo "help" | timeout 5 "$BINARY" >/dev/null 2>&1; then
    echo -e "${GREEN}✓${NC}"
else
    echo -e "${RED}✗${NC}"
    exit 1
fi

# Test 3: Echo command
echo -n "Testing echo command... "
output=$(echo 'echo "Hello World"' | timeout 5 "$BINARY" 2>/dev/null | tail -n 1)
if [ "$output" = "Hello World" ]; then
    echo -e "${GREEN}✓${NC}"
else
    echo -e "${RED}✗${NC}"
    echo "Expected: 'Hello World', Got: '$output'"
    exit 1
fi

# Test 4: Variable expansion
echo -n "Testing variable expansion... "
result=$(echo -e 'export TEST_VAR=success\necho $TEST_VAR' | timeout 5 "$BINARY" 2>/dev/null | tail -n 1)
if [ "$result" = "success" ]; then
    echo -e "${GREEN}✓${NC}"
else
    echo -e "${RED}✗${NC}"
    echo "Expected: 'success', Got: '$result'"
    exit 1
fi

# Test 5: Array creation
echo -n "Testing array creation... "
if echo 'array test_array=(one two three)' | timeout 5 "$BINARY" >/dev/null 2>&1; then
    echo -e "${GREEN}✓${NC}"
else
    echo -e "${RED}✗${NC}"
    exit 1
fi

# Test 6: Exit code
echo -n "Testing exit codes... "
if echo "exit 42" | timeout 5 "$BINARY" >/dev/null 2>&1; then
    exit_code=$?
    if [ $exit_code -eq 42 ]; then
        echo -e "${GREEN}✓${NC}"
    else
        echo -e "${RED}✗${NC}"
        echo "Expected exit code 42, got $exit_code"
        exit 1
    fi
else
    echo -e "${RED}✗${NC}"
    exit 1
fi

echo
echo -e "${GREEN}All basic tests passed! 🎉${NC}"
echo
echo "To run Leizi Shell interactively:"
echo "  $BINARY"
echo
echo "For more tests, check the tests/ directory (when implemented)"