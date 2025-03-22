#!/bin/bash

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Starting local build check...${NC}\n"

# Function to print section header
print_section() {
    echo -e "\n${YELLOW}=== $1 ===${NC}\n"
}

# Clean build directory
print_section "Cleaning build directory"
rm -rf build
mkdir build
cd build

# Normal build and test
print_section "Building project"
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j

# Copy compilation database to root for clang-tidy
cp compile_commands.json ..

print_section "Running tests"
ctest --output-on-failure

# Build with coverage
print_section "Building with coverage"
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
make clean
make -j
ctest --output-on-failure

if command -v lcov >/dev/null 2>&1; then
    print_section "Generating coverage report"
    lcov --directory . --capture --output-file coverage.info
    lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info
    lcov --list coverage.info
else
    echo -e "${YELLOW}lcov not found, skipping coverage report${NC}"
fi

# Static analysis
cd ..
print_section "Running static analysis"

if command -v clang-tidy >/dev/null 2>&1; then
    echo "Running clang-tidy..."
    # Use the compilation database from project root
    find src include -name '*.cpp' -o -name '*.h' | \
    while read file; do
        echo "Checking $file"
        clang-tidy -p . "$file" \
            -checks='-*,readability-*,modernize-*,performance-*,bugprone-*,-llvmlibc-*' \
            -- -I$(pwd)/include
    done
else
    echo -e "${YELLOW}clang-tidy not found, skipping static analysis${NC}"
fi

# Build packages
print_section "Building packages"
cd build
cpack -G "TGZ"

echo -e "\n${GREEN}All checks completed!${NC}"

# Print summary of generated files
print_section "Generated files in build directory"
ls -lh *.tar.gz 2>/dev/null || echo "No packages generated"
ls -lh *.a 2>/dev/null || echo "No static libraries generated"
ls -lh coverage.info 2>/dev/null || echo "No coverage report generated"

echo -e "\n${GREEN}Done!${NC}"
