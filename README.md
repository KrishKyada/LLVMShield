# warp_aai - Educational LLVM Obfuscation Toolchain (MVP)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Educational Purpose](https://img.shields.io/badge/Purpose-Educational-blue.svg)](https://github.com)

A minimal, safe **educational** MVP demonstrating LLVM-based code obfuscation techniques. This toolchain provides a complete pipeline from C/C++ source code to obfuscated native binaries with comprehensive reporting.

## ⚠️ ETHICAL USAGE AND DISCLAIMER

**EDUCATIONAL PURPOSE ONLY** - This tool is designed for educational and research purposes to demonstrate basic code obfuscation techniques using LLVM infrastructure.

### Acceptable Uses:
- Learning LLVM pass development and code transformation techniques
- Research in software protection and reverse engineering
- Academic coursework and computer science education
- Legitimate software protection (with proper disclosure to users)

### Prohibited Uses:
- Malware development or distribution
- Circumventing security measures without authorization
- Any malicious or harmful activities
- Violation of applicable laws or regulations

**Users are solely responsible for compliance with all applicable laws and ethical guidelines.**

## Features

The warp_aai toolchain implements several **basic** obfuscation techniques:

1. **String XOR Encryption**: Encrypts string constants using XOR with a configurable key
2. **Bogus Function Insertion**: Adds fake functions with meaningless arithmetic operations
3. **Symbol Renaming**: Renames private global symbols with "_obf" suffix
4. **Dead Code Insertion**: Inserts harmless dead conditional branches

**Important**: These are **simple, educational** transformations that are easily reversible and not suitable for production use.

## Prerequisites

### Required Tools
- **LLVM/Clang toolchain** (version 10-14, preferably 12)
- **CMake** (version 3.10 or higher)
- **Python** 3.8 or higher
- **C++ compiler** with C++14 support (GCC/Clang)

### Platform Support
- **Primary**: Linux (Ubuntu, CentOS, etc.)
- **Cross-compilation**: Windows targets via MinGW (optional)
- **macOS**: Should work but not extensively tested

### Installing Dependencies

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install llvm-12-dev clang-12 cmake python3 build-essential
# Optional: for Windows cross-compilation
sudo apt-get install mingw-w64
```

#### CentOS/RHEL:
```bash
sudo dnf install llvm-devel clang cmake python3 gcc-c++
```

#### macOS (with Homebrew):
```bash
brew install llvm cmake python3
export PATH="/usr/local/opt/llvm/bin:$PATH"
```

## Building the Toolchain

### 1. Clone or Download the Repository

```bash
git clone <repository-url>
cd warp_aai
```

### 2. Build the LLVM Pass Plugin

```bash
# Create build directory
mkdir build && cd build

# Configure build (may need to specify LLVM_DIR)
cmake ..

# If LLVM is installed in a non-standard location:
# cmake -DLLVM_DIR=/path/to/llvm/lib/cmake/llvm ..

# Build the plugin
make

# The plugin will be built as: build/lib/libSimpleObfPass.so (Linux)
#                              build/lib/libSimpleObfPass.dylib (macOS)  
#                              build/lib/SimpleObfPass.dll (Windows)
```

### 3. Verify Installation

```bash
# Test that the plugin loads correctly
opt -load build/lib/libSimpleObfPass.so -help | grep simple-obf

# Should show: -simple-obf - Educational obfuscation pass (warp_aai MVP)
```

## Usage

### Basic Usage

Make sure the Python wrapper is executable:
```bash
chmod +x warp_aai.py
```

Run the toolchain on the example program:
```bash
./warp_aai.py example.c --pass-lib build/lib/libSimpleObfPass.so
```

### Advanced Usage Examples

#### Custom Parameters:
```bash
./warp_aai.py example.c \
    --pass-lib build/lib/libSimpleObfPass.so \
    --xor-key 42 \
    --bogus-count 5 \
    --cycles 2 \
    --out my_obfuscated_app \
    --verbose
```

#### Multiple Source Files:
```bash
./warp_aai.py src/main.c src/utils.c src/crypto.c \
    --pass-lib build/lib/libSimpleObfPass.so \
    --out multi_file_app \
    --xor-key 123 \
    --bogus-count 3
```

#### Cross-compile for Windows:
```bash
./warp_aai.py example.c \
    --pass-lib build/lib/libSimpleObfPass.so \
    --target windows \
    --out example.exe \
    --verbose
```

### Command Line Options

```
usage: warp_aai.py [-h] --pass-lib PASS_LIB [--out OUTPUT] [--xor-key XOR_KEY]
                   [--bogus-count BOGUS_COUNT] [--cycles CYCLES]
                   [--target {linux,windows}] [--verbose] [--keep-temp]
                   input_files [input_files ...]

positional arguments:
  input_files           Input C/C++ source files

optional arguments:
  --pass-lib PASS_LIB   Path to compiled obfuscation pass library
  --out OUTPUT          Output binary filename (default: obfuscated_binary)
  --xor-key XOR_KEY     XOR key for string encryption (default: 170)
  --bogus-count BOGUS_COUNT
                        Number of bogus functions to insert (default: 2)
  --cycles CYCLES       Number of obfuscation cycles (default: 1)
  --target {linux,windows}
                        Target platform (default: linux)
  --verbose, -v         Enable verbose output
  --keep-temp           Keep temporary files for debugging
```

## Output and Reporting

### Console Output Example

```
[INFO] Using: clang version 12.0.1
=== Step 1: Compiling to LLVM bitcode ===
[INFO] Compiling example.c -> warp_aai_work/example.bc
=== Step 2: Linking bitcode ===
=== Step 3: Running obfuscation pass ===
[INFO] Running obfuscation pass: warp_aai_work/linked.bc -> warp_aai_work/obfuscated.bc
[INFO] Parameters: xor_key=170, bogus_count=2, cycles=1
[INFO] Pass output:
  [warp_aai] Starting obfuscation pass...
  [warp_aai] Running cycle 1/1
  [warp_aai] Encrypted string: This is a secret message that should be obfuscated! (len=54)
  [warp_aai] Encrypted string: warp_aai Educational Obfuscation Demo (len=37)
  [warp_aai] Encrypted string: Version 1.0.0 - Educational MVP (len=32)
  [warp_aai] Inserted bogus function: bogus_func_0_1234
  [warp_aai] Inserted bogus function: bogus_func_1_5678
  [warp_aai] Added dead conditional to function: main
=== Step 4: Compiling to native binary ===
[INFO] Compiling to native binary: warp_aai_work/obfuscated.bc -> obfuscated_binary
=== Step 5: Generating report ===
=== Obfuscation Complete ===
[INFO] Input files: 1
[INFO] Output binary: obfuscated_binary (8736 bytes)
[INFO] Strings obfuscated: 3
[INFO] Fake functions added: 2
[INFO] Cycles completed: 1
[INFO] Report saved: warp_report_1640995200.json
```

### JSON Report Example

The toolchain generates a comprehensive JSON report (e.g., `warp_report_1640995200.json`):

```json
{
  "warp_aai_version": "1.0.0-mvp",
  "timestamp": "2021-12-31T23:59:60.000000",
  "execution_time_seconds": 2.34,
  "input_files": [
    {
      "path": "/home/user/warp_aai/example.c",
      "size_bytes": 3456,
      "modified_time": "2021-12-31T12:34:56.000000"
    }
  ],
  "parameters": {
    "xor_key": 170,
    "bogus_count": 2,
    "cycles": 1,
    "target": "linux",
    "pass_library": "/home/user/warp_aai/build/lib/libSimpleObfPass.so"
  },
  "output": {
    "path": "/home/user/warp_aai/obfuscated_binary",
    "size_bytes": 8736,
    "target": "linux"
  },
  "obfuscation_results": {
    "strings_obfuscated": 3,
    "fake_functions_inserted": 2,
    "cycles_completed": 1,
    "xor_key_used": 170,
    "bogus_functions_requested": 2
  },
  "methods_applied": [
    "XOR string encryption",
    "Bogus function insertion", 
    "Private symbol renaming",
    "Dead conditional branch insertion"
  ],
  "limitations": [
    "Educational MVP - not production ready",
    "Simple XOR encryption (easily reversible)",
    "Minimal control flow changes",
    "No runtime unpacking or advanced anti-analysis"
  ],
  "notes": "This is an educational tool demonstrating basic LLVM-based obfuscation techniques."
}
```

## Testing

### Quick Test

Run the included test script:
```bash
./test.sh
```

### Manual Testing

1. **Build the toolchain**:
   ```bash
   mkdir build && cd build
   cmake .. && make
   cd ..
   ```

2. **Run on example**:
   ```bash
   ./warp_aai.py example.c --pass-lib build/lib/libSimpleObfPass.so --verbose
   ```

3. **Execute the obfuscated binary**:
   ```bash
   ./obfuscated_binary
   ```

4. **Verify obfuscation** by comparing with original:
   ```bash
   clang example.c -o example_original
   ./example_original
   # Compare output and binary sizes
   ```

## Architecture and Implementation Details

### Pipeline Overview

1. **Source → Bitcode**: C/C++ files are compiled to LLVM bitcode (.bc)
2. **Bitcode Linking**: Multiple .bc files are linked into a single module
3. **Obfuscation Pass**: Custom LLVM pass applies transformations
4. **Bitcode → Native**: Final compilation to target platform binary

### LLVM Pass Implementation

The core obfuscation logic is in `SimpleObfPass.cpp`:

- **ModulePass**: Operates on entire LLVM modules
- **Command-line Integration**: Uses `llvm::cl::opt` for parameter passing
- **Safe Transformations**: Avoids breaking ABI or program semantics
- **Telemetry Output**: Generates machine-readable statistics

### Security Considerations

⚠️ **Important**: This is an **educational MVP** with significant limitations:

- **XOR encryption** is trivially reversible
- **Bogus functions** are obviously fake to any analyst  
- **Symbol renaming** is cosmetic only
- **Control flow changes** are minimal and predictable
- **No anti-debugging** or runtime protection

**Real-world obfuscation** requires much more sophisticated techniques.

## Troubleshooting

### Common Issues

1. **"opt: command not found"**
   - Install LLVM development tools
   - Ensure LLVM bin directory is in PATH

2. **"Plugin loading failed"**
   - Verify LLVM version compatibility (10-14 supported)
   - Check that plugin was built successfully
   - Try absolute path to plugin library

3. **"Cross-compilation failed"**
   - Install MinGW for Windows cross-compilation
   - May fall back to native compilation with warning

4. **Build errors with LLVM**
   - Specify LLVM_DIR explicitly: `cmake -DLLVM_DIR=/path/to/llvm/cmake ..`
   - Ensure C++14 compiler support

### Debug Mode

Keep temporary files for analysis:
```bash
./warp_aai.py example.c --pass-lib build/lib/libSimpleObfPass.so --keep-temp --verbose
```

Examine intermediate files in `warp_aai_work/`:
- `example.bc` - Original bitcode
- `linked.bc` - Linked bitcode  
- `obfuscated.bc` - Obfuscated bitcode
- `warp_pass_telemetry.json` - Pass statistics

## Extending the Toolchain

### Adding New Obfuscation Techniques

1. **Modify `SimpleObfPass.cpp`**:
   - Add new transformation method
   - Update `runOnModule()` to call it
   - Add command-line parameters if needed

2. **Update telemetry**:
   - Add counters for new technique
   - Update JSON output in `outputTelemetry()`

3. **Update wrapper**:
   - Add new parameters to `warp_aai.py`
   - Update report generation

### Example: Adding Instruction Substitution

```cpp
// In SimpleObfPass.cpp
bool substituteInstructions(Module &M) {
    // Replace add instructions with equivalent sub + neg
    // This is just an example - add proper implementation
    return false;
}

// In runOnModule():
changed |= substituteInstructions(M);
```

## Contributing

This is an educational project. Contributions should focus on:

- **Educational value**: Clear, well-documented code
- **Safety**: No malicious or harmful transformations
- **Compatibility**: Support for common LLVM versions
- **Documentation**: Comprehensive explanations

## License and Legal

### License

This project is licensed under the **MIT License**:

```
MIT License

Copyright (c) 2024 warp_aai Educational Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### Legal Disclaimer

- This software is provided for **educational purposes only**
- Users are **solely responsible** for legal compliance
- The authors assume **no liability** for misuse
- Check your local laws before using this software
- Do not use for any **malicious or harmful** purposes

## Acknowledgments

- **LLVM Project**: For providing the infrastructure for code analysis and transformation
- **Educational Community**: For promoting responsible learning of security techniques  
- **Open Source Contributors**: For making tools like this possible

## TODO and Limitations

This is an educational MVP with several important limitations:

### Current Limitations:
- **Simple XOR encryption** (easily reversible)
- **Minimal control flow obfuscation**
- **No runtime unpacking or decryption**
- **No anti-debugging features**
- **Limited LLVM version testing**

### Future Educational Enhancements:
- More sophisticated string encryption
- Better control flow obfuscation examples
- Function call indirection demonstrations
- Constant array obfuscation
- Educational anti-analysis techniques

### Production Considerations:
Real-world obfuscation tools would require:
- **Runtime decryption** of strings and code
- **Anti-debugging** and **anti-VM** techniques
- **Code virtualization** and **packing**
- **Tamper detection** and **integrity checks**
- **Performance optimization** for obfuscated code

---

**Remember**: This tool is for **education only**. Use responsibly and ethically!