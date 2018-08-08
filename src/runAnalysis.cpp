#include <LLVMHeaders.h>
#include "llvmEstimator.h"
#include "FunctionInfoPass.h"

void runEstimatorAnalysis(llvm::Function& f, llvm::PassBuilder::OptimizationLevel optLevel)
{
  llvm::FunctionAnalysisManager FAM = runDefaultOptimization(f, optLevel);
  auto &FA = FAM.getResult<FunctionInfoPass>(f);
  llvm::outs() << "----------------------------------------\n";
  FA.printAnalysis();
  llvm::outs() << "----------------------------------------\n";
}

