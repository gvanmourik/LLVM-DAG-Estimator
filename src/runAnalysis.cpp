#include <LLVMHeaders.h>
#include "FunctionInfoPass.h"

void runEstimatorAnalysis(std::unique_ptr<llvm::Module>& module, llvm::FunctionAnalysisManager* FAM)
{
	for (auto iter=module->begin(); iter!=module->end(); ++iter)
	{
    auto &FA = FAM->getResult<FunctionInfoPass>(*iter);
    llvm::outs() << "----------------------------------------\n";
		FA.printAnalysis();
    llvm::outs() << "----------------------------------------\n";
	}
}

