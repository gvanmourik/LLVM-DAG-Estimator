#include <LLVMHeaders.h>
#include "FunctionInfoPass.h"

llvm::PassBuilder::OptimizationLevel getOptLevel(int optLevel_int)
{
  char optLevel = '0' + optLevel_int;
  switch (optLevel){
    case '1':
      return llvm::PassBuilder::O1;
    case '2':
      return llvm::PassBuilder::O2;
    case '3':
      return llvm::PassBuilder::O3;
    case 's':
      return llvm::PassBuilder::Os;
    case 'z':
      return llvm::PassBuilder::Oz;
    default:
      std::cerr << "Got invalid opt level " << optLevel << " - defaulting to 2" << std::endl;
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

  llvm::ModuleAnalysisManager MAM(DebugAM);
  llvm::CGSCCAnalysisManager CGAM(DebugAM);
  llvm::FunctionAnalysisManager FAM(DebugAM);
  llvm::LoopAnalysisManager LAM(DebugAM);

	auto FPM = passBuilder.buildFunctionSimplificationPipeline(
	 	optLevel, 
    llvm::PassBuilder::ThinLTOPhase::None,
	 	DebugPM);

  /// Register the passes used in the simplification pipeline
	passBuilder.crossRegisterProxies(LAM, FAM, CGAM, MAM);

	/// Custom analysis passes
  FAM.registerPass([&]{ return FunctionInfoPass(); });
	// MAM->registerPass([&]{ return ModuleInfoPass(FAM); });

	passBuilder.registerFunctionAnalyses(FAM);
	passBuilder.registerLoopAnalyses(LAM);

  // llvm::outs() << f;
  FPM.run(f, FAM);

  return FAM;
}

