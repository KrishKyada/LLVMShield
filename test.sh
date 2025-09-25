#!/bin/bash

#
# test.sh - warp_aai Educational Obfuscation Toolchain Test Script
#
# This script demonstrates the complete end-to-end usage of the warp_aai
# toolchain, including building, obfuscating, and testing the example program.
#
# EDUCATIONAL PURPOSE ONLY - This tool is for learning LLVM-based obfuscation.
#

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_banner() {
    echo "=========================================="
    echo "  warp_aai Educational Obfuscation Test"
    echo "=========================================="
    echo ""
}

# Check if required tools are available
check_dependencies() {
    print_step "Checking dependencies..."
    
    local missing=()
    
    for tool in clang opt llvm-link cmake python3; do
        if ! command -v "$tool" &> /dev/null; then
            missing+=("$tool")
        fi
    done
    
    if [ ${#missing[@]} -ne 0 ]; then
        print_error "Missing required tools: ${missing[*]}"
        print_error "Please install LLVM/Clang development tools and CMake"
        return 1
    fi
    
    # Show versions
    print_info "Using clang: $(clang --version | head -1)"
    print_info "Using cmake: $(cmake --version | head -1)"
    print_info "Using python: $(python3 --version)"
    
    return 0
}

# Build the LLVM pass plugin
build_plugin() {
    print_step "Building LLVM pass plugin..."
    
    # Create build directory
    if [ ! -d "build" ]; then
        mkdir build
    fi
    
    cd build
    
    # Configure and build
    if ! cmake ..; then
        print_error "CMake configuration failed"
        cd ..
        return 1
    fi
    
    if ! make; then
        print_error "Build failed"
        cd ..
        return 1
    fi
    
    cd ..
    
    # Verify plugin was built
    if [ -f "build/lib/libSimpleObfPass.so" ]; then
        print_info "Plugin built successfully: build/lib/libSimpleObfPass.so"
    elif [ -f "build/lib/libSimpleObfPass.dylib" ]; then
        print_info "Plugin built successfully: build/lib/libSimpleObfPass.dylib" 
    elif [ -f "build/lib/SimpleObfPass.dll" ]; then
        print_info "Plugin built successfully: build/lib/SimpleObfPass.dll"
    else
        print_error "Plugin build failed - no library found"
        return 1
    fi
    
    return 0
}

# Test plugin loading
test_plugin_loading() {
    print_step "Testing plugin loading..."
    
    # Determine plugin file
    local plugin=""
    if [ -f "build/lib/libSimpleObfPass.so" ]; then
        plugin="build/lib/libSimpleObfPass.so"
    elif [ -f "build/lib/libSimpleObfPass.dylib" ]; then
        plugin="build/lib/libSimpleObfPass.dylib"
    elif [ -f "build/lib/SimpleObfPass.dll" ]; then
        plugin="build/lib/SimpleObfPass.dll"
    else
        print_error "No plugin library found"
        return 1
    fi
    
    # Test loading
    if opt -load "$plugin" -help 2>/dev/null | grep -q "simple-obf"; then
        print_info "Plugin loads correctly and registers the pass"
    else
        print_error "Plugin failed to load or register"
        return 1
    fi
    
    return 0
}

# Compile original example for comparison
compile_original() {
    print_step "Compiling original example for comparison..."
    
    if ! clang example.c -o example_original; then
        print_error "Failed to compile original example"
        return 1
    fi
    
    print_info "Original example compiled: example_original"
    return 0
}

# Run the obfuscation toolchain
run_obfuscation() {
    print_step "Running warp_aai obfuscation toolchain..."
    
    # Determine plugin file
    local plugin=""
    if [ -f "build/lib/libSimpleObfPass.so" ]; then
        plugin="build/lib/libSimpleObfPass.so"
    elif [ -f "build/lib/libSimpleObfPass.dylib" ]; then
        plugin="build/lib/libSimpleObfPass.dylib"
    elif [ -f "build/lib/SimpleObfPass.dll" ]; then
        plugin="build/lib/SimpleObfPass.dll"
    fi
    
    # Make Python script executable
    chmod +x warp_aai.py
    
    # Run with verbose output and custom parameters
    if ! python3 warp_aai.py example.c \
        --pass-lib "$plugin" \
        --xor-key 42 \
        --bogus-count 3 \
        --cycles 1 \
        --out example_obfuscated \
        --verbose; then
        print_error "Obfuscation failed"
        return 1
    fi
    
    print_info "Obfuscation completed successfully"
    return 0
}

# Test the obfuscated binary
test_obfuscated_binary() {
    print_step "Testing obfuscated binary..."
    
    if [ ! -f "example_obfuscated" ]; then
        print_error "Obfuscated binary not found"
        return 1
    fi
    
    # Make executable
    chmod +x example_obfuscated
    
    print_info "Running original example:"
    echo "--- Original Output ---"
    if ! ./example_original; then
        print_warning "Original example failed to run"
    fi
    
    echo ""
    print_info "Running obfuscated example:"
    echo "--- Obfuscated Output ---"
    if ! ./example_obfuscated; then
        print_error "Obfuscated example failed to run"
        return 1
    fi
    
    return 0
}

# Compare binary sizes and show statistics
show_statistics() {
    print_step "Showing statistics and report..."
    
    # Show file sizes
    if [ -f "example_original" ] && [ -f "example_obfuscated" ]; then
        local orig_size=$(stat -c%s example_original 2>/dev/null || stat -f%z example_original)
        local obf_size=$(stat -c%s example_obfuscated 2>/dev/null || stat -f%z example_obfuscated)
        
        print_info "Binary size comparison:"
        echo "  Original:   $orig_size bytes"
        echo "  Obfuscated: $obf_size bytes"
        
        if [ "$obf_size" -gt "$orig_size" ]; then
            local increase=$((obf_size - orig_size))
            print_info "Size increase: +$increase bytes"
        else
            print_info "No significant size increase"
        fi
    fi
    
    # Show report if available
    local report_file=$(ls warp_report_*.json 2>/dev/null | head -1)
    if [ -n "$report_file" ]; then
        print_info "Generated report: $report_file"
        
        # Extract key statistics from JSON report
        if command -v python3 &> /dev/null; then
            echo ""
            print_info "Obfuscation statistics:"
            python3 -c "
import json
try:
    with open('$report_file', 'r') as f:
        data = json.load(f)
    results = data.get('obfuscation_results', {})
    print(f'  Strings obfuscated: {results.get(\"strings_obfuscated\", 0)}')
    print(f'  Fake functions inserted: {results.get(\"fake_functions_inserted\", 0)}')
    print(f'  Cycles completed: {results.get(\"cycles_completed\", 0)}')
    print(f'  XOR key used: {results.get(\"xor_key_used\", 0)}')
    print(f'  Execution time: {data.get(\"execution_time_seconds\", 0)} seconds')
except Exception as e:
    print(f'Could not parse report: {e}')
"
        fi
    fi
    
    return 0
}

# Cleanup temporary files
cleanup() {
    print_step "Cleaning up temporary files..."
    
    # Remove temporary work directory
    if [ -d "warp_aai_work" ]; then
        rm -rf warp_aai_work
        print_info "Cleaned up work directory"
    fi
    
    # Remove test binaries (optional)
    read -p "Remove test binaries (example_original, example_obfuscated)? [y/N]: " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -f example_original example_obfuscated
        print_info "Test binaries removed"
    fi
}

# Main test function
main() {
    print_banner
    
    print_info "This script will test the complete warp_aai toolchain:"
    print_info "1. Check dependencies"
    print_info "2. Build LLVM pass plugin"
    print_info "3. Test plugin loading"  
    print_info "4. Compile original example"
    print_info "5. Run obfuscation pipeline"
    print_info "6. Test obfuscated binary"
    print_info "7. Show statistics"
    print_info "8. Clean up"
    echo ""
    
    read -p "Continue? [Y/n]: " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Nn]$ ]]; then
        print_info "Test cancelled by user"
        exit 0
    fi
    
    echo ""
    
    # Run test steps
    local steps=(
        "check_dependencies"
        "build_plugin"
        "test_plugin_loading"
        "compile_original"
        "run_obfuscation"
        "test_obfuscated_binary"
        "show_statistics"
    )
    
    local step_names=(
        "Checking dependencies"
        "Building plugin"
        "Testing plugin loading"
        "Compiling original"
        "Running obfuscation"
        "Testing obfuscated binary"
        "Showing statistics"
    )
    
    for i in "${!steps[@]}"; do
        echo ""
        if ! "${steps[$i]}"; then
            print_error "Step failed: ${step_names[$i]}"
            print_error "Test script aborted"
            exit 1
        fi
        print_info "✓ ${step_names[$i]} completed successfully"
    done
    
    echo ""
    print_step "All tests passed successfully!"
    echo ""
    print_info "The warp_aai toolchain is working correctly."
    print_info "You can now use it to obfuscate your own C/C++ programs."
    print_info ""
    print_info "Remember: This is an EDUCATIONAL tool only!"
    print_info "Use responsibly and ethically."
    
    echo ""
    cleanup
    
    echo ""
    print_info "Test script completed."
}

# Run main function
main "$@"