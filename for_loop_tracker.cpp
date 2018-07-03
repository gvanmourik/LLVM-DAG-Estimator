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
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "LoopAnalysisPass.h"


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
	FPM->doInitialization();

	/// Returned function
	Function *ForLoopFnc = GenForLoop( context, builder, module, N );
	
	/// Transform passes	
	outs() << "--------------------BEFORE-----------------------\n" << *module << "\n"; //Before passes
	FPM->run(*ForLoopFnc);
	outs() << "---------------------AFTER-----------------------\n" << *module << "\n"; //After passes

	/// Analysis passes
	FPM = make_unique<legacy::FunctionPassManager>(module);
	FPM->add( createLoopAnalysisPass() ); //custom pass
	FPM->doInitialization();
	FPM->run(*ForLoopFnc);

	/// Create a JIT
	std::string collectedErrors;
	ExecutionEngine *engine = 
		EngineBuilder(std::move(mainModule))
		.setErrorStr(&collectedErrors)
		.setEngineKind(EngineKind::JIT)
		.create();

	/// Execution Engine
	if ( !engine )
	{
		std::string error = "Unable to construct execution engine: " + collectedErrors;
		perror(error.c_str());
		return -1;
	}

	/// Output IR module
	// outs() << "\n" << *module;


	return 0;
}

Function* GenForLoop(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters)
{
	Function *ForLoopFnc = 
		cast<Function>( module->getOrInsertFunction("ForLoopFnc", Type::getInt32Ty(context)) );

	Value* N = ConstantInt::get(builder.getInt32Ty(), iters);
	Value* zero = ConstantInt::get(builder.getInt32Ty(), 0);
	Value* one = ConstantInt::get(builder.getInt32Ty(), 1);

	/// BBs
	BasicBlock *EntryBB = BasicBlock::Create(context, "entry", ForLoopFnc);
	BasicBlock *ForLoop1EntryBB = BasicBlock::Create(context, "forLoop1Entry", ForLoopFnc);
	BasicBlock *ForLoop1BodyBB = BasicBlock::Create(context, "forLoop1Body", ForLoopFnc);
		BasicBlock *ForLoop2EntryBB = BasicBlock::Create(context, "forLoop2Entry", ForLoopFnc);
		BasicBlock *ForLoop2BodyBB = BasicBlock::Create(context, "forLoop2Body", ForLoopFnc);
		BasicBlock *ForLoop2ExitBB = BasicBlock::Create(context, "forLoop2Exit", ForLoopFnc);
	BasicBlock *ForLoop1ExitBB = BasicBlock::Create(context, "forLoop1Exit", ForLoopFnc);
	BasicBlock *ExitBB = BasicBlock::Create(context, "exit", ForLoopFnc);
	
	/// Variables
	Value *ifiLTN, *ifjLTN, *i, *j, *iVal, *jVal, *counter, *counterVal, *returnValue;


	/// EntryBB
	builder.SetInsertPoint(EntryBB);
	i = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "i");
	j = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "j");
	counter = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "counter");
	builder.CreateStore(zero, i);
	builder.CreateStore(zero, j);
	builder.CreateStore(zero, counter);
	builder.CreateBr(ForLoop1EntryBB);

	/// ForLoop1EntryBB
	builder.SetInsertPoint(ForLoop1EntryBB);
	iVal = builder.CreateLoad(i, "iVal");
	ifiLTN = builder.CreateICmpULT(iVal, N, "ifiLTN");
	builder.CreateCondBr(ifiLTN, ForLoop1BodyBB, ForLoop1ExitBB);
	
	/// ForLoop1BodyBB
	builder.SetInsertPoint(ForLoop1BodyBB);
	iVal = builder.CreateAdd(iVal, one);
	builder.CreateStore(iVal, i);
	builder.CreateBr(ForLoop2EntryBB);

		/// ForLoop2EntryBB
		builder.SetInsertPoint(ForLoop2EntryBB);
		jVal = builder.CreateLoad(j, "jVal");
		ifjLTN = builder.CreateICmpULT(jVal, N, "ifjLTN");
		builder.CreateCondBr(ifjLTN, ForLoop2BodyBB, ForLoop2ExitBB);

		/// ForLoop2BodyBB
		builder.SetInsertPoint(ForLoop2BodyBB);
		counterVal = builder.CreateLoad(counter, "counterVal");
		jVal = builder.CreateAdd(jVal, one);
		counterVal = builder.CreateAdd(counterVal, one);
		builder.CreateStore(jVal, j);
		builder.CreateStore(counterVal, counter);
		builder.CreateBr(ForLoop2EntryBB);

		/// ForLoop2ExitBB
		builder.SetInsertPoint(ForLoop2ExitBB);
		builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "dummyAlloca");
		builder.CreateBr(ForLoop1EntryBB);
	
	/// ForLoop1ExitBB
	builder.SetInsertPoint(ForLoop1ExitBB);
	builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "dummyAlloca");
	builder.CreateBr(ExitBB);

	/// ExitBB
	builder.SetInsertPoint(ExitBB);
	returnValue = builder.CreateLoad(counter, "finalCount");
	ReturnInst::Create(context, returnValue, ExitBB);


	return ForLoopFnc;
}


