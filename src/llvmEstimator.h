#include <LLVMHeaders.h>

llvm::PassBuilder::OptimizationLevel setOptLevel(std::string &optLevelString);
llvm::ExecutionEngine *buildExecutionEngine(std::unique_ptr<llvm::Module> &module);
llvm::TargetMachine *buildTargetMachine();

llvm::Function* generateTest1(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
                              llvm::Module* module, int iters);
llvm::Function* generateTest2(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
                              llvm::Module* module, int iters, llvm::Function *callee);
llvm::Function* generateTest3(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
                              llvm::Module* module, int iters);

std::unique_ptr<llvm::Module> readIRText(llvm::StringRef text);
std::unique_ptr<llvm::Module> readIRFile(llvm::StringRef fname);

