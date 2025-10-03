#!/bin/bash
# Performance benchmark script for Leizi Shell

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
LEIZI_BIN="$BUILD_DIR/leizi"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Leizi Shell Performance Benchmark ===${NC}\n"

# Check if binary exists
if [ ! -f "$LEIZI_BIN" ]; then
    echo -e "${RED}Error: leizi binary not found at $LEIZI_BIN${NC}"
    echo "Please build the project first: cd build && cmake .. && make"
    exit 1
fi

# Benchmark 1: Startup time
echo -e "${YELLOW}[1/4] Measuring startup time...${NC}"
STARTUP_TIMES=()
for i in {1..10}; do
    START=$(date +%s%N)
    echo "exit" | timeout 2 "$LEIZI_BIN" > /dev/null 2>&1 || true
    END=$(date +%s%N)
    ELAPSED=$(( (END - START) / 1000000 )) # Convert to milliseconds
    STARTUP_TIMES+=($ELAPSED)
done

# Calculate average startup time
TOTAL=0
for time in "${STARTUP_TIMES[@]}"; do
    TOTAL=$((TOTAL + time))
done
AVG_STARTUP=$((TOTAL / ${#STARTUP_TIMES[@]}))

if [ $AVG_STARTUP -lt 50 ]; then
    echo -e "${GREEN}✓ Startup time: ${AVG_STARTUP}ms (target: <50ms)${NC}"
else
    echo -e "${RED}✗ Startup time: ${AVG_STARTUP}ms (target: <50ms)${NC}"
fi

# Benchmark 2: Prompt generation time
echo -e "\n${YELLOW}[2/4] Measuring prompt generation time...${NC}"
PROMPT_TIMES=()
for i in {1..10}; do
    START=$(date +%s%N)
    # Simulate prompt generation by running version command
    echo "version" | timeout 2 "$LEIZI_BIN" > /dev/null 2>&1 || true
    END=$(date +%s%N)
    ELAPSED=$(( (END - START) / 1000000 ))
    PROMPT_TIMES+=($ELAPSED)
done

TOTAL=0
for time in "${PROMPT_TIMES[@]}"; do
    TOTAL=$((TOTAL + time))
done
AVG_PROMPT=$((TOTAL / ${#PROMPT_TIMES[@]}))

if [ $AVG_PROMPT -lt 100 ]; then
    echo -e "${GREEN}✓ Prompt generation: ${AVG_PROMPT}ms (target: <100ms)${NC}"
else
    echo -e "${RED}✗ Prompt generation: ${AVG_PROMPT}ms (target: <100ms)${NC}"
fi

# Benchmark 3: Command execution overhead
echo -e "\n${YELLOW}[3/4] Measuring command execution overhead...${NC}"
EXEC_TIMES=()
for i in {1..5}; do
    START=$(date +%s%N)
    echo "echo test" | timeout 2 "$LEIZI_BIN" > /dev/null 2>&1 || true
    END=$(date +%s%N)
    ELAPSED=$(( (END - START) / 1000000 ))
    EXEC_TIMES+=($ELAPSED)
done

TOTAL=0
for time in "${EXEC_TIMES[@]}"; do
    TOTAL=$((TOTAL + time))
done
AVG_EXEC=$((TOTAL / ${#EXEC_TIMES[@]}))

echo -e "${GREEN}✓ Command execution: ${AVG_EXEC}ms${NC}"

# Benchmark 4: Memory usage
echo -e "\n${YELLOW}[4/4] Measuring memory usage...${NC}"
if command -v /usr/bin/time &> /dev/null; then
    # Run leizi and measure memory
    MEM_OUTPUT=$(/usr/bin/time -l echo "version" | timeout 2 "$LEIZI_BIN" 2>&1 > /dev/null | grep "maximum resident set size" || echo "0")
    if [[ $MEM_OUTPUT =~ ([0-9]+) ]]; then
        MEM_KB=$((${BASH_REMATCH[1]} / 1024))
        echo -e "${GREEN}✓ Peak memory usage: ${MEM_KB} KB${NC}"
    else
        echo -e "${YELLOW}⚠ Could not measure memory usage${NC}"
    fi
else
    echo -e "${YELLOW}⚠ /usr/bin/time not available for memory measurement${NC}"
fi

# Summary
echo -e "\n${YELLOW}=== Benchmark Summary ===${NC}"
echo "Startup time:     ${AVG_STARTUP}ms"
echo "Prompt gen:       ${AVG_PROMPT}ms"
echo "Command exec:     ${AVG_EXEC}ms"

# Overall assessment
if [ $AVG_STARTUP -lt 50 ] && [ $AVG_PROMPT -lt 100 ]; then
    echo -e "\n${GREEN}✓ All performance targets met!${NC}"
    exit 0
else
    echo -e "\n${YELLOW}⚠ Some performance targets not met${NC}"
    exit 1
fi
