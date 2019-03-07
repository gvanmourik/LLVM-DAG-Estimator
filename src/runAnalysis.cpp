#include <LLVMHeaders.h>
#include "llvmEstimator.h"
#include "FunctionInfoPass.h"

//use to set the extern flag declared FunctionInfoPass.h
bool generateDOT;

void runEstimatorAnalysis(llvm::Function& f, llvm::PassBuilder::OptimizationLevel optLevel, bool dotFlag)
{
  llvm::FunctionAnalysisManager FAM = runDefaultOptimization(f, optLevel);
  generateDOT = dotFlag;
  auto &FA = FAM.getResult<FunctionInfoPass>(f);
  if (generateDOT)
  {
  	llvm::outs() << " -- Convert to image files by:\n";
  	llvm::outs() << "    -- Navigating to dotFiles/ with: cd ../../dotFiles\n";
  	llvm::outs() << "    -- Running: ../scripts/dot_to_png\n";
  }
  llvm::outs() << "----------------------------------------\n";
  FA.printAnalysis();
  llvm::outs() << "----------------------------------------\n";
}

