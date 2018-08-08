#include <LLVMHeaders.h>
#include "FunctionInfoPass.h"

llvm::PassBuilder::OptimizationLevel getOptLevel(char optLevel)
{
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
      return llvm::PassBuilder::Os;
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
  llvm::FunctionAnalysisManager FAM(DebugAM);
  llvm::LoopAnalysisManager LAM(DebugAM);

	auto FPM = passBuilder.buildFunctionSimplificationPipeline(
	 	optLevel, 
    llvm::PassBuilder::ThinLTOPhase::None,
	 	DebugPM);

	/// Manually register the proxies used in the pipeline
	// FAM->registerPass([&]{ return LoopAnalysisManagerFunctionProxy(LAM); });
	// LAM->registerPass([&]{ return FunctionAnalysisManagerLoopProxy(FAM); });
	// passBuilder.crossRegisterProxies(LAM, FAM, CGAM, MAM);

	/// Use the below method to register all added transform passes rather than 
	/// passing in each individual pass as a lambda to register that pass.
	/// [ example: FAM->registerPass([&]{ return SROA(); }); ]
	// passBuilder.registerFunctionAnalyses(FAM);
	// passBuilder.registerLoopAnalyses(LAM);

	/// Custom analysis passes
	// FAM->registerPass([&]{ return LoopInfoAnalysisPass(); });
  FAM.registerPass([&]{ return FunctionInfoPass(); });
	// MAM->registerPass([&]{ return ModuleInfoPass(FAM); });

	// passBuilder.registerModuleAnalyses(MAM);
	passBuilder.registerFunctionAnalyses(FAM);
	passBuilder.registerLoopAnalyses(LAM);

  // llvm::outs() << "--------------------BEFORE-----------------------\n" << *module << "\n"; //print before
  FPM.run(f, FAM);
  // llvm::outs() << "---------------------AFTER-----------------------\n" << *module << "\n"; //print after

  return FAM;
}

