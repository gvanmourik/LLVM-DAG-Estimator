#include <LLVMHeaders.h>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>


int main(int argc, char* argv[])
{
	/// User inputs
	if ( argv[1] == nullptr )
	{
		perror("Missing the loop iterations argument [integer value]");
		return -1;
	}
	int N = atoi(argv[1]);
	if ( argv[2] == nullptr )
	{
		perror("Missing the optimization level argument [O1, O2, O3, Os, Oz]");
		return -1;
	}
	std::string optLevelString = argv[2];
	PassBuilder::OptimizationLevel optLevel = setOptLevel(optLevelString);


	/// LLVM IR Variables
  LLVMContext context;
  IRBuilder<> builder(context);
	std::unique_ptr<Module> mainModule;
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();

  runEstimatorAnalysis(module, FAM);

	return 0;
}







