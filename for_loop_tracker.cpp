#include <llvm/ADT/APInt.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include <llvm/Transforms/InstCombine/InstCombine.h>
// #include "LoopAnalysis.h"


using namespace llvm;


/// Function declarations
Function* GenForLoop(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters);


int main(int argc, char* argv[])
{
	int N;
	std::cout << "Enter the number of loop iterations: ";
	std::cin >> N;

	/// LLVM IR Variables
	static LLVMContext context;
	static IRBuilder<> builder(context);
	std::unique_ptr<Module> mainModule( new Module("ForLoopFncModule", context) );
	Module *module = mainModule.get();
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();

	/// Function pass manager setup
	static std::unique_ptr<legacy::FunctionPassManager> FPM;
	FPM = make_unique<legacy::FunctionPassManager>(module);
	FPM->add( createInstructionCombiningPass(true) );
	FPM->add( createReassociatePass() );
	// FPM->add( LoopAnalysis() ); //custom pass
	FPM->doInitialization();

	/// Returned function
	Function *ForLoopFnc = GenForLoop( context, builder, module, N );
		
	outs() << "--------------------BEFORE-----------------------\n" << *module << "\n"; //Before passes
	FPM->run(*ForLoopFnc);
	outs() << "---------------------AFTER-----------------------\n" << *module << "\n"; //After passes


	/// Create a JIT
	std::string collectedErrors;
	ExecutionEngine *engine = 
		EngineBuilder(std::move(mainModule))
		.setErrorStr(&collectedErrors)
		// .setUseMCJIT(true)
		.setEngineKind(EngineKind::JIT)
		.create();

	/// Execution Engine
	if ( !engine )
	{
		std::string error = "Unable to construct execution engine: " + collectedErrors;
		perror(error.c_str());
		return -1;
	}

	std::vector<GenericValue> Args(0); // Empty vector as no args are passed
	GenericValue value = engine->runFunction(ForLoopFnc, Args);
	// Loop::Body* function_ptr = (Loop::Body*)engine->getFunctionAddress(ForLoopFnc->getName());
	// if (function_ptr == nullptr)
	// {
	// 	errs() << "Failed to collect the compiled function: " << ForLoopFnc->getName() << "\n";
	// 	return 1;
	// }

	/// Output IR module
	// outs() << "\n" << *module;


	return 0;
}

Function* GenForLoop(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters)
{
	Function *ForLoopFnc = 
		cast<Function>( module->getOrInsertFunction("ForLoopFnc", Type::getInt32Ty(context)) );
	// FunctionType *funcType = FunctionType::get(builder.getInt32Ty(), args, false);
	// Function *ForLoopFnc = Function::Create(funcType, Function::ExternalLinkage, "ForLoopFncFunc", module);

	Value* N = ConstantInt::get(builder.getInt32Ty(), iters);
	Value* zero = ConstantInt::get(builder.getInt32Ty(), 0);
	Value* one = ConstantInt::get(builder.getInt32Ty(), 1);

	/// BBs
	BasicBlock *EntryBB = BasicBlock::Create(context, "entry", ForLoopFnc);
	BasicBlock *ForLoopEntryBB = BasicBlock::Create(context, "forLoopEntry", ForLoopFnc);
	BasicBlock *ForLoopBodyBB = BasicBlock::Create(context, "forLoopBody", ForLoopFnc);
	BasicBlock *ForLoopExitBB = BasicBlock::Create(context, "forLoopExit", ForLoopFnc);
	BasicBlock *ExitBB = BasicBlock::Create(context, "exit", ForLoopFnc);
	
	/// Variables
	Value *ifIndexLTN, *index, *indexVal, *returnValue;


	/// EntryBB
	builder.SetInsertPoint(EntryBB);
	index = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "index");
	builder.CreateStore(zero, index);
	builder.CreateBr(ForLoopEntryBB);

	/// ForLoopEntryBB
	builder.SetInsertPoint(ForLoopEntryBB);
	indexVal = builder.CreateLoad(index, "indexVal");
	ifIndexLTN = builder.CreateICmpULT(indexVal, N, "ifIndexLTN");
	builder.CreateCondBr(ifIndexLTN, ForLoopBodyBB, ForLoopExitBB); //end of EntryBB
	
	/// ForLoopBodyBB
	builder.SetInsertPoint(ForLoopBodyBB);
	indexVal = builder.CreateAdd(indexVal, one);
	builder.CreateStore(indexVal, index);
	builder.CreateBr(ForLoopEntryBB);
	
	/// ForLoopExitBB
	builder.SetInsertPoint(ForLoopExitBB);
	builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "dummyAlloca");
	builder.CreateBr(ExitBB);

	/// ExitBB
	builder.SetInsertPoint(ExitBB);
	returnValue = builder.CreateLoad(index, "finalIndex");
	ReturnInst::Create(context, returnValue, ExitBB);


	return ForLoopFnc;
}