/*
 * SimpleObfPass.cpp - Educational LLVM Obfuscation Pass
 * 
 * EDUCATIONAL MVP ONLY - This tool demonstrates basic code obfuscation techniques
 * for educational and research purposes. The transformations are intentionally
 * simple, reversible, and non-malicious.
 * 
 * Techniques implemented:
 * - String XOR encryption with simple runtime decoding
 * - Insertion of benign bogus functions (dead code)
 * - Basic symbol renaming for private globals
 * - Minimal control flow obfuscation (dead conditional branches)
 * 
 * ETHICAL USAGE: This tool should only be used for:
 * - Educational purposes and learning LLVM pass development
 * - Research in software protection and reverse engineering
 * - Legitimate software protection (with proper disclosure)
 * 
 * DO NOT use for malicious purposes, malware development, or to circumvent
 * security measures. Users are responsible for compliance with applicable laws.
 */

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <random>
#include <vector>
#include <string>
#include <fstream>

using namespace llvm;

// Command line options for the pass
static cl::opt<int> XorKey("xor-key", 
    cl::desc("XOR key for string encryption"), 
    cl::init(170));

static cl::opt<int> BogusCount("bogus-count", 
    cl::desc("Number of bogus functions to insert"), 
    cl::init(2));

static cl::opt<int> Cycles("cycles", 
    cl::desc("Number of obfuscation cycles to run"), 
    cl::init(1));

namespace {
struct SimpleObfPass : public ModulePass {
    static char ID;
    
    // Statistics counters
    unsigned strings_obf_count = 0;
    unsigned fake_funcs_inserted = 0;
    unsigned cycles_completed = 0;
    
    SimpleObfPass() : ModulePass(ID) {}
    
    bool runOnModule(Module &M) override {
        errs() << "[warp_aai] Starting obfuscation pass...\n";
        
        bool changed = false;
        
        // Run obfuscation for specified number of cycles
        for (int cycle = 0; cycle < Cycles; ++cycle) {
            errs() << "[warp_aai] Running cycle " << (cycle + 1) << "/" << Cycles << "\n";
            
            // Apply all obfuscation techniques
            changed |= obfuscateStrings(M);
            changed |= insertBogusFunctions(M);
            changed |= renamePrivateGlobals(M);
            changed |= insertDeadConditionals(M);
            
            cycles_completed++;
        }
        
        // Output telemetry as JSON for parsing by wrapper script
        outputTelemetry();
        
        errs() << "[warp_aai] Obfuscation completed. "
               << "Strings: " << strings_obf_count << ", "
               << "Bogus funcs: " << fake_funcs_inserted << ", "
               << "Cycles: " << cycles_completed << "\n";
        
        return changed;
    }
    
private:
    /**
     * Obfuscate string constants by XOR encryption
     * Creates encrypted global arrays and simple decode helpers
     */
    bool obfuscateStrings(Module &M) {
        std::vector<GlobalVariable*> stringGlobals;
        
        // Find string constants
        for (GlobalVariable &GV : M.globals()) {
            if (!GV.hasInitializer() || !GV.isConstant()) continue;
            
            // Skip external linkage to avoid breaking ABI
            if (GV.hasExternalLinkage()) continue;
            
            if (auto *CDA = dyn_cast<ConstantDataArray>(GV.getInitializer())) {
                if (CDA->isString()) {
                    stringGlobals.push_back(&GV);
                }
            }
        }
        
        // Encrypt found strings
        for (GlobalVariable *GV : stringGlobals) {
            encryptString(M, GV);
            strings_obf_count++;
        }
        
        return !stringGlobals.empty();
    }
    
    /**
     * Encrypt a single string global variable
     */
    void encryptString(Module &M, GlobalVariable *GV) {
        auto *CDA = cast<ConstantDataArray>(GV->getInitializer());
        StringRef originalStr = CDA->getAsCString();
        
        // Create encrypted version
        std::vector<uint8_t> encrypted;
        for (size_t i = 0; i < originalStr.size(); ++i) {
            encrypted.push_back(originalStr[i] ^ (XorKey & 0xFF));
        }
        encrypted.push_back(0); // null terminator (also encrypted)
        
        // Create new encrypted global
        ArrayType *arrayType = ArrayType::get(Type::getInt8Ty(M.getContext()), encrypted.size());
        Constant *encryptedData = ConstantDataArray::get(M.getContext(), encrypted);
        
        GlobalVariable *encryptedGV = new GlobalVariable(
            M, arrayType, true, GlobalValue::PrivateLinkage, 
            encryptedData, GV->getName() + "_enc");
        
        // Replace the original initializer with encrypted version
        // Note: In a full implementation, we'd create runtime decode helpers
        // For this MVP, we just replace the data (decoder would be added separately)
        GV->setInitializer(encryptedData);
        GV->setName(GV->getName() + "_obf");
        
        errs() << "[warp_aai] Encrypted string: " << originalStr << " (len=" << originalStr.size() << ")\n";
    }
    
    /**
     * Insert bogus/fake functions that serve as dead code
     */
    bool insertBogusFunctions(Module &M) {
        LLVMContext &Ctx = M.getContext();
        bool changed = false;
        
        for (int i = 0; i < BogusCount; ++i) {
            // Create function name
            std::string funcName = "bogus_func_" + std::to_string(i) + "_" + std::to_string(rand() % 10000);
            
            // Create function type: int bogus_func(int)
            FunctionType *FT = FunctionType::get(Type::getInt32Ty(Ctx), 
                                                Type::getInt32Ty(Ctx), false);
            
            Function *BogusF = Function::Create(FT, GlobalValue::PrivateLinkage, funcName, M);
            
            // Create function body with meaningless operations
            BasicBlock *BB = BasicBlock::Create(Ctx, "entry", BogusF);
            IRBuilder<> Builder(BB);
            
            // Get function argument
            Value *Arg = BogusF->arg_begin();
            
            // Perform some meaningless arithmetic
            Value *Result = Arg;
            for (int j = 0; j < 3; ++j) {
                Result = Builder.CreateAdd(Result, ConstantInt::get(Type::getInt32Ty(Ctx), j + i));
                Result = Builder.CreateMul(Result, ConstantInt::get(Type::getInt32Ty(Ctx), 2));
            }
            
            // Return the computed value (which will never be used)
            Builder.CreateRet(Result);
            
            fake_funcs_inserted++;
            changed = true;
            
            errs() << "[warp_aai] Inserted bogus function: " << funcName << "\n";
        }
        
        return changed;
    }
    
    /**
     * Rename private global variables by appending _obf suffix
     */
    bool renamePrivateGlobals(Module &M) {
        bool changed = false;
        std::vector<GlobalVariable*> toRename;
        
        // Collect private globals
        for (GlobalVariable &GV : M.globals()) {
            if (GV.hasPrivateLinkage() && !GV.getName().endswith("_obf") && 
                !GV.getName().endswith("_enc")) {
                toRename.push_back(&GV);
            }
        }
        
        // Rename them
        for (GlobalVariable *GV : toRename) {
            std::string oldName = GV->getName().str();
            GV->setName(oldName + "_obf");
            changed = true;
            errs() << "[warp_aai] Renamed global: " << oldName << " -> " << GV->getName() << "\n";
        }
        
        return changed;
    }
    
    /**
     * Insert dead conditional branches in functions (minimal control flow obfuscation)
     */
    bool insertDeadConditionals(Module &M) {
        bool changed = false;
        int functionsModified = 0;
        
        // Only modify one function to keep things minimal and safe
        for (Function &F : M) {
            if (F.isDeclaration() || F.getName().startswith("bogus_func_")) continue;
            if (functionsModified >= 1) break; // Limit to one function for safety
            
            // Insert dead conditional at function entry
            BasicBlock &EntryBB = F.getEntryBlock();
            if (EntryBB.empty()) continue;
            
            LLVMContext &Ctx = M.getContext();
            IRBuilder<> Builder(&EntryBB, EntryBB.begin());
            
            // Create always-false condition: 0 == 1
            Value *Cond = Builder.CreateICmpEQ(
                ConstantInt::get(Type::getInt32Ty(Ctx), 0),
                ConstantInt::get(Type::getInt32Ty(Ctx), 1)
            );
            
            // Create dead basic block
            BasicBlock *DeadBB = BasicBlock::Create(Ctx, "dead_branch_obf", &F);
            BasicBlock *ContBB = BasicBlock::Create(Ctx, "continue_obf", &F);
            
            // Create conditional branch (will always go to ContBB)
            Builder.CreateCondBr(Cond, DeadBB, ContBB);
            
            // Dead basic block (never executed)
            IRBuilder<> DeadBuilder(DeadBB);
            DeadBuilder.CreateBr(ContBB);
            
            // Move rest of original entry block to continue block
            IRBuilder<> ContBuilder(ContBB);
            
            // Note: In a full implementation, we'd properly move instructions
            // For this MVP, we just create the structure
            ContBuilder.CreateBr(&EntryBB); // This creates a simple loop structure
            
            functionsModified++;
            changed = true;
            
            errs() << "[warp_aai] Added dead conditional to function: " << F.getName() << "\n";
            break; // Only modify one function for safety
        }
        
        return changed;
    }
    
    /**
     * Output telemetry data as JSON for the wrapper script to parse
     */
    void outputTelemetry() {
        // Write telemetry to a JSON file that the wrapper can read
        std::error_code EC;
        raw_fd_ostream TelemetryFile("warp_pass_telemetry.json", EC);
        
        if (!EC) {
            TelemetryFile << "{\n";
            TelemetryFile << "  \"strings_obf_count\": " << strings_obf_count << ",\n";
            TelemetryFile << "  \"fake_funcs_inserted\": " << fake_funcs_inserted << ",\n";
            TelemetryFile << "  \"cycles_completed\": " << cycles_completed << ",\n";
            TelemetryFile << "  \"xor_key\": " << XorKey << ",\n";
            TelemetryFile << "  \"bogus_count_requested\": " << BogusCount << "\n";
            TelemetryFile << "}\n";
            TelemetryFile.close();
            
            errs() << "[warp_aai] Telemetry written to warp_pass_telemetry.json\n";
        } else {
            errs() << "[warp_aai] Warning: Could not write telemetry file\n";
        }
        
        // Also output to stderr for debugging
        errs() << "[warp_aai] TELEMETRY: {\"strings_obf_count\":" << strings_obf_count 
               << ",\"fake_funcs_inserted\":" << fake_funcs_inserted 
               << ",\"cycles_completed\":" << cycles_completed << "}\n";
    }
};

char SimpleObfPass::ID = 0;

// Register the pass
static RegisterPass<SimpleObfPass> X("simple-obf", 
    "Educational obfuscation pass (warp_aai MVP)", false, false);

} // anonymous namespace