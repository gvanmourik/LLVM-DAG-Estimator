#include <LLVMHeaders.h>

/**
 * @brief getOptLevel
 * @param optLevel A char ('0', '1', '2', '3', 's', 'z') defining the opt level
 * @return An LLVM enum for the desired optimization
 */
llvm::PassBuilder::OptimizationLevel getOptLevel(std::string optLevel_str);

llvm::ExecutionEngine *buildExecutionEngine(std::unique_ptr<llvm::Module> &module);
llvm::TargetMachine *buildTargetMachine();

llvm::Function* generateTest1(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
                              llvm::Module* module, int iters);
llvm::Function* generateTest2(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
                              llvm::Module* module, int iters, llvm::Function *callee);
llvm::Function* generateTest3(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
                              llvm::Module* module, int iters);

std::unique_ptr<llvm::Module> readIRText(llvm::StringRef text, llvm::LLVMContext& context);
std::unique_ptr<llvm::Module> readIRFile(llvm::StringRef fname, llvm::LLVMContext& context);

llvm::FunctionAnalysisManager
runDefaultOptimization(llvm::Function& f, llvm::PassBuilder::OptimizationLevel optLevel);

void runEstimatorAnalysis(llvm::Function& f, llvm::PassBuilder::OptimizationLevel optLevel);
