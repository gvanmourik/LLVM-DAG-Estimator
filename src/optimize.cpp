#include <LLVMHeaders.h>
#include <stdlib.h>
#include "FunctionInfoPass.h"

llvm::PassBuilder::OptimizationLevel getOptLevel(std::string optLevel_str)
{
  if (optLevel_str == "0") 
    return llvm::PassBuilder::O0;
  else if (optLevel_str == "1")
    return llvm::PassBuilder::O1;
  else if (optLevel_str == "2")
    return llvm::PassBuilder::O2;
  else if (optLevel_str == "3")
    return llvm::PassBuilder::O3;
  else if (optLevel_str == "s") 
    return llvm::PassBuilder::Os;
  else if (optLevel_str == "z")
      return llvm::PassBuilder::Oz;
  else {
    std::cerr << "Got invalid opt level " << optLevel_str << " - defaulting to O2" << std::endl;
    return llvm::PassBuilder::O2;
  }
}


llvm::FunctionAnalysisManager
runDefaultOptimization(llvm::Function& f, llvm::PassBuilder::OptimizationLevel optLevel)
{
	/// Function analysis and pass managers
	bool DebugPM = false;
	bool DebugAM = false;

  llvm::PassBuilder passBuilder;
  llvm::FunctionPassManager FPM(DebugPM);

  llvm::ModuleAnalysisManager MAM(DebugAM);
  llvm::CGSCCAnalysisManager CGAM(DebugAM);
  llvm::FunctionAnalysisManager FAM(DebugAM);
  llvm::LoopAnalysisManager LAM(DebugAM);

  if ( optLevel != llvm::PassBuilder::O0 )
  {
    FPM = passBuilder.buildFunctionSimplificationPipeline(
    optLevel, 
    llvm::PassBuilder::ThinLTOPhase::None,
    DebugPM);
  }
	

  /// Register the passes used in the simplification pipeline
	passBuilder.crossRegisterProxies(LAM, FAM, CGAM, MAM);

	/// Custom analysis passes
  FAM.registerPass([&]{ return FunctionInfoPass(); });
	// MAM->registerPass([&]{ return ModuleInfoPass(FAM); });

	passBuilder.registerFunctionAnalyses(FAM);
	passBuilder.registerLoopAnalyses(LAM);

  if ( optLevel != llvm::PassBuilder::O0) {
    FPM.run(f, FAM);
  }

  return FAM;
}

