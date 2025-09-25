#!/usr/bin/env python3
"""
warp_aai.py - Educational LLVM Obfuscation Toolchain Wrapper

EDUCATIONAL MVP ONLY - This wrapper demonstrates a complete obfuscation pipeline
using LLVM tools and a custom obfuscation pass. For educational use only.

ETHICAL USAGE: This tool should only be used for:
- Educational purposes and learning LLVM-based transformations
- Research in software protection and reverse engineering
- Legitimate software protection (with proper disclosure)

DO NOT use for malicious purposes. Users are responsible for compliance
with applicable laws and regulations.
"""

import argparse
import subprocess
import os
import sys
import json
import time
from pathlib import Path
from datetime import datetime
import shutil

class WarpAAIToolchain:
    """Main toolchain orchestrator for the warp_aai obfuscation pipeline"""
    
    def __init__(self):
        self.verbose = False
        self.temp_files = []
        self.start_time = time.time()
        
    def log(self, message, level="INFO"):
        """Log messages with timestamp"""
        if level == "ERROR":
            print(f"[ERROR] {message}", file=sys.stderr)
        elif self.verbose or level == "INFO":
            print(f"[{level}] {message}")
    
    def check_dependencies(self):
        """Verify required LLVM tools are available"""
        required_tools = ['clang', 'opt', 'llvm-link']
        missing = []
        
        for tool in required_tools:
            if not shutil.which(tool):
                missing.append(tool)
        
        if missing:
            self.log(f"Missing required tools: {', '.join(missing)}", "ERROR")
            self.log("Please install LLVM development tools (version 10-14 recommended)", "ERROR")
            return False
        
        # Check tool versions
        try:
            result = subprocess.run(['clang', '--version'], 
                                  capture_output=True, text=True)
            self.log(f"Using: {result.stdout.split()[0]} {result.stdout.split()[2]}")
        except:
            pass
            
        return True
    
    def compile_to_bitcode(self, source_files, output_dir):
        """Compile C/C++ source files to LLVM bitcode"""
        bitcode_files = []
        
        for source in source_files:
            if not os.path.exists(source):
                raise FileNotFoundError(f"Source file not found: {source}")
            
            # Generate bitcode filename
            base_name = Path(source).stem
            bc_file = os.path.join(output_dir, f"{base_name}.bc")
            
            self.log(f"Compiling {source} -> {bc_file}")
            
            # Compile to bitcode
            cmd = [
                'clang', '-emit-llvm', '-c', '-o', bc_file, source,
                '-O1',  # Light optimization to generate cleaner IR
            ]
            
            result = subprocess.run(cmd, capture_output=True, text=True)
            if result.returncode != 0:
                raise RuntimeError(f"Compilation failed for {source}:\n{result.stderr}")
            
            bitcode_files.append(bc_file)
            self.temp_files.append(bc_file)
        
        return bitcode_files
    
    def link_bitcode(self, bitcode_files, output_file):
        """Link multiple bitcode files into one"""
        if len(bitcode_files) == 1:
            # Single file, just copy
            shutil.copy2(bitcode_files[0], output_file)
            return output_file
        
        self.log(f"Linking {len(bitcode_files)} bitcode files -> {output_file}")
        
        cmd = ['llvm-link'] + bitcode_files + ['-o', output_file]
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode != 0:
            raise RuntimeError(f"Bitcode linking failed:\n{result.stderr}")
        
        self.temp_files.append(output_file)
        return output_file
    
    def run_obfuscation_pass(self, input_bc, output_bc, pass_lib, xor_key, bogus_count, cycles):
        """Run the custom obfuscation pass"""
        self.log(f"Running obfuscation pass: {input_bc} -> {output_bc}")
        self.log(f"Parameters: xor_key={xor_key}, bogus_count={bogus_count}, cycles={cycles}")
        
        # Construct opt command
        cmd = [
            'opt',
            '-load', pass_lib,
            '-simple-obf',
            f'-xor-key={xor_key}',
            f'-bogus-count={bogus_count}',
            f'-cycles={cycles}',
            input_bc,
            '-o', output_bc
        ]
        
        # Run the pass
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode != 0:
            raise RuntimeError(f"Obfuscation pass failed:\n{result.stderr}")
        
        # Log pass output (contains telemetry and debug info)
        if result.stderr:
            self.log("Pass output:")
            for line in result.stderr.strip().split('\n'):
                if line.strip():
                    self.log(f"  {line}")
        
        self.temp_files.append(output_bc)
        return output_bc
    
    def compile_to_native(self, input_bc, output_binary, target="linux"):
        """Compile obfuscated bitcode to native binary"""
        self.log(f"Compiling to native binary: {input_bc} -> {output_binary}")
        
        if target == "windows":
            # Cross-compile for Windows using mingw
            compiler = "x86_64-w64-mingw32-clang"
            if not shutil.which(compiler):
                self.log("Windows cross-compiler not found, falling back to clang", "WARNING")
                compiler = "clang"
        else:
            compiler = "clang"
        
        cmd = [compiler, input_bc, '-o', output_binary]
        
        # Add target-specific flags
        if target == "windows" and compiler.endswith("mingw32-clang"):
            cmd.extend(['-target', 'x86_64-w64-mingw32'])
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode != 0:
            raise RuntimeError(f"Native compilation failed:\n{result.stderr}")
        
        return output_binary
    
    def parse_telemetry(self, working_dir):
        """Parse telemetry data from the pass"""
        telemetry_file = os.path.join(working_dir, "warp_pass_telemetry.json")
        
        if os.path.exists(telemetry_file):
            try:
                with open(telemetry_file, 'r') as f:
                    return json.load(f)
            except json.JSONDecodeError as e:
                self.log(f"Warning: Could not parse telemetry file: {e}", "WARNING")
        
        # Fallback: return empty telemetry
        return {
            "strings_obf_count": 0,
            "fake_funcs_inserted": 0,
            "cycles_completed": 0,
            "xor_key": 0,
            "bogus_count_requested": 0
        }
    
    def generate_report(self, input_files, output_file, telemetry, parameters):
        """Generate final JSON report"""
        # Get output file size
        output_size = os.path.getsize(output_file) if os.path.exists(output_file) else 0
        
        # Create report
        report = {
            "warp_aai_version": "1.0.0-mvp",
            "timestamp": datetime.now().isoformat(),
            "execution_time_seconds": round(time.time() - self.start_time, 2),
            "input_files": [
                {
                    "path": os.path.abspath(f),
                    "size_bytes": os.path.getsize(f),
                    "modified_time": datetime.fromtimestamp(os.path.getmtime(f)).isoformat()
                }
                for f in input_files
            ],
            "parameters": parameters,
            "output": {
                "path": os.path.abspath(output_file),
                "size_bytes": output_size,
                "target": parameters.get("target", "linux")
            },
            "obfuscation_results": {
                "strings_obfuscated": telemetry.get("strings_obf_count", 0),
                "fake_functions_inserted": telemetry.get("fake_funcs_inserted", 0),
                "cycles_completed": telemetry.get("cycles_completed", 0),
                "xor_key_used": telemetry.get("xor_key", 0),
                "bogus_functions_requested": telemetry.get("bogus_count_requested", 0)
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
        
        return report
    
    def cleanup(self):
        """Clean up temporary files"""
        for temp_file in self.temp_files:
            try:
                if os.path.exists(temp_file):
                    os.remove(temp_file)
                    self.log(f"Cleaned up: {temp_file}")
            except Exception as e:
                self.log(f"Warning: Could not clean up {temp_file}: {e}", "WARNING")
    
    def run(self, args):
        """Main execution pipeline"""
        self.verbose = args.verbose
        
        try:
            # Check dependencies
            if not self.check_dependencies():
                return 1
            
            # Validate inputs
            if not args.pass_lib or not os.path.exists(args.pass_lib):
                self.log(f"Pass library not found: {args.pass_lib}", "ERROR")
                return 1
            
            # Create working directory
            work_dir = "warp_aai_work"
            os.makedirs(work_dir, exist_ok=True)
            
            # Store parameters for report
            parameters = {
                "xor_key": args.xor_key,
                "bogus_count": args.bogus_count,
                "cycles": args.cycles,
                "target": args.target,
                "pass_library": os.path.abspath(args.pass_lib)
            }
            
            # Step 1: Compile to bitcode
            self.log("=== Step 1: Compiling to LLVM bitcode ===")
            bitcode_files = self.compile_to_bitcode(args.input_files, work_dir)
            
            # Step 2: Link bitcode
            self.log("=== Step 2: Linking bitcode ===")
            linked_bc = os.path.join(work_dir, "linked.bc")
            self.link_bitcode(bitcode_files, linked_bc)
            
            # Step 3: Run obfuscation pass
            self.log("=== Step 3: Running obfuscation pass ===")
            obfuscated_bc = os.path.join(work_dir, "obfuscated.bc")
            self.run_obfuscation_pass(
                linked_bc, obfuscated_bc, args.pass_lib,
                args.xor_key, args.bogus_count, args.cycles
            )
            
            # Step 4: Compile to native
            self.log("=== Step 4: Compiling to native binary ===")
            self.compile_to_native(obfuscated_bc, args.output, args.target)
            
            # Step 5: Parse telemetry and generate report
            self.log("=== Step 5: Generating report ===")
            telemetry = self.parse_telemetry(work_dir)
            report = self.generate_report(args.input_files, args.output, telemetry, parameters)
            
            # Write report file
            report_file = f"warp_report_{int(time.time())}.json"
            with open(report_file, 'w') as f:
                json.dump(report, f, indent=2)
            
            # Print summary
            self.log("=== Obfuscation Complete ===")
            self.log(f"Input files: {len(args.input_files)}")
            self.log(f"Output binary: {args.output} ({report['output']['size_bytes']} bytes)")
            self.log(f"Strings obfuscated: {telemetry.get('strings_obf_count', 0)}")
            self.log(f"Fake functions added: {telemetry.get('fake_funcs_inserted', 0)}")
            self.log(f"Cycles completed: {telemetry.get('cycles_completed', 0)}")
            self.log(f"Report saved: {report_file}")
            
            return 0
            
        except Exception as e:
            self.log(f"Error: {e}", "ERROR")
            return 1
        
        finally:
            # Clean up temporary files
            if not args.keep_temp:
                self.cleanup()


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="warp_aai - Educational LLVM Obfuscation Toolchain (MVP)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s example.c --pass-lib build/lib/libSimpleObfPass.so
  %(prog)s src/*.c --xor-key 42 --bogus-count 5 --cycles 2 --out my_app
  %(prog)s test.c --target windows --out test.exe --verbose

EDUCATIONAL USE ONLY - Do not use for malicious purposes.
        """
    )
    
    parser.add_argument('input_files', nargs='+', 
                      help='Input C/C++ source files')
    
    parser.add_argument('--pass-lib', required=True,
                      help='Path to compiled obfuscation pass library')
    
    parser.add_argument('--out', dest='output', default='obfuscated_binary',
                      help='Output binary filename (default: obfuscated_binary)')
    
    parser.add_argument('--xor-key', type=int, default=170,
                      help='XOR key for string encryption (default: 170)')
    
    parser.add_argument('--bogus-count', type=int, default=2,
                      help='Number of bogus functions to insert (default: 2)')
    
    parser.add_argument('--cycles', type=int, default=1,
                      help='Number of obfuscation cycles (default: 1)')
    
    parser.add_argument('--target', choices=['linux', 'windows'], default='linux',
                      help='Target platform (default: linux)')
    
    parser.add_argument('--verbose', '-v', action='store_true',
                      help='Enable verbose output')
    
    parser.add_argument('--keep-temp', action='store_true',
                      help='Keep temporary files for debugging')
    
    args = parser.parse_args()
    
    # Create and run toolchain
    toolchain = WarpAAIToolchain()
    return toolchain.run(args)


if __name__ == "__main__":
    sys.exit(main())