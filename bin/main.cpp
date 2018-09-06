#include <LLVMHeaders.h>
#include "llvmEstimator.h"
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <CLI/CLI.hpp>


int main(int argc, char* argv[])
{
  std::string optLevel_str = "2";
  std::string cppFname;
  std::string irFname;
  std::string fxnName;

  int builtinTest = -1;
  int iters = 5;

  CLI::App app{"LLVM Estimator"};
  app.add_option("--opt", optLevel_str, "the load balancer type to use");
  app.add_option("--cpp", cppFname, "A C++ file to generate IR from");
  app.add_option("--ir", irFname, "An IR file to read in");
  app.add_option("--builtin", builtinTest, "The number of the built-in test to run");
  app.add_option("--fxn", fxnName, "The name of the function in the IR file to analyze");
  app.add_option("--iters,-i", iters, "The number of iterations for buildint tests 1-3");
  
  if (argc > 1){
    try {
      app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
      int rc = app.exit(e);
      exit(rc);
    }
  }

  llvm::LLVMContext context;
  llvm::PassBuilder::OptimizationLevel llvmOptLevel = getOptLevel(optLevel_str);
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  bool ranTest = false;
  if (!cppFname.empty()){
    std::cerr << "Do not yet support generated LLVM functions directly from C++ source" << std::endl;
    ranTest = true;
    return 1;
  }

  if (!irFname.empty()){
    ranTest = true;
    std::unique_ptr<llvm::Module> module = readIRFile(irFname, context);
    for (auto iter=module->begin(); iter!=module->end(); ++iter){
      llvm::Function& f = *iter;
      if (fxnName.empty()){ //no specific request
        runEstimatorAnalysis(f, llvmOptLevel);
      } 
      else if (fxnName == f.getName()) {
        runEstimatorAnalysis(f, llvmOptLevel);
      }
    }
    if (!fxnName.empty()) {
      std::cerr << fxnName << " not found in module" << std::endl;
    }
  }

  if (builtinTest >= 0){
    llvm::IRBuilder<> builder(context);
    llvm::Function* f;
    llvm::Function* callee;
    auto module = llvm::make_unique<llvm::Module>("TestModule", context);
    switch(builtinTest){
      case 1:
        f = generateTest1(context, builder, module.get(), iters);
        llvm::outs() << *module.get();
        break;
      case 2:
        callee = generateTest1(context, builder, module.get(), iters);
        f = generateTest2(context, builder, module.get(), iters, callee);
        break;
      case 3:
        f = generateTest3(context, builder, module.get(), iters);
        break;
      default:
        std::cerr << "Invalid test number " << builtinTest << " specified" << std::endl;
        return 1;
    }
    runEstimatorAnalysis(*f, llvmOptLevel);
    ranTest = true;
  }

  if (!ranTest){
    std::cerr << app.help() << std::endl;
    std::cerr << "No IR, C++, or built-in test specified" << std::endl;
    return 1;
  }

	return 0;
}







