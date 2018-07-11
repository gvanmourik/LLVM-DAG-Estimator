#include <llvm/ADT/APInt.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
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
// #include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/CFGPrinter.h>
#include <llvm/Analysis/InstructionSimplify.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/SROA.h>
#include <llvm/Transforms/Utils/SimplifyInstructions.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

/// Custom analysis passes
#include "LoopInfoAnalysisPass.h"

using namespace llvm;


/// Function declarations
Function* GenForLoop(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters);
void print(Analysis_t &analysis);


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

	// Function analysis and pass managers
	bool DebugPM = true;
	bool DebugAM = true;
	static FunctionPassManager *FPM = 
		new FunctionPassManager(DebugPM);
	static FunctionAnalysisManager *FAM = 
		new FunctionAnalysisManager(DebugAM);

	// ModulePassManager *MPM = 
	// 	new ModulePassManager(DebugPM);
	// ModuleAnalysisManager *MAM = 
	// 	new ModuleAnalysisManager(DebugAM);


	PassBuilder passBuilder;
	*FPM = passBuilder.buildFunctionSimplificationPipeline(
		PassBuilder::OptimizationLevel::O2, 
		PassBuilder::ThinLTOPhase::None, 
		DebugPM);

	// *MPM = passBuilder.buildModuleOptimizationPipeline(
	// 	PassBuilder::OptimizationLevel::O2,
	// 	DebugPM);


	static LoopAnalysisManager *LAM = 
		new LoopAnalysisManager(DebugAM);
	FAM->registerPass([&]{ return LoopAnalysisManagerFunctionProxy(*LAM); });
	// static FunctionAnalysisManager const& FAM_const = *FAM;
	// FAM->registerPass([&]{ return FunctionAnalysisManagerLoopProxy(*FAM); });
	// FPM->addPass( RequireAnalysisPass<LoopAnalysisManagerFunctionProxy, Function>() );

	/// Use the below method to register all added transform passes rather than 
	/// passing in each individual pass as a lambda to register that pass.
	/// [ example: FAM->registerPass([&]{ return SROA(); }); ]
	passBuilder.registerFunctionAnalyses(*FAM);
	// passBuilder.registerModuleAnalyses(*MAM);

	/// Returned IR function
	Function *ForLoopFnc = GenForLoop( context, builder, module, N );
	
	/// Transform passes	
	outs() << "--------------------BEFORE-----------------------\n" << *module << "\n"; //print before
	FPM->run(*ForLoopFnc, *FAM);
	outs() << "---------------------AFTER-----------------------\n" << *module << "\n"; //print after


	// /// Custom analysis pass
	// FPM->addPass( RequireAnalysisPass<LoopInfoAnalysisPass, Function>() );
	// passBuilder.registerFunctionAnalyses(*FAM);
	// // FAM->registerPass([&]{ return LoopInfoAnalysisPass(); });

	// /// Collect and print result
	// auto &result = FAM->getResult<LoopInfoAnalysisPass>(*ForLoopFnc);
	// print(result); //prints in reverse order

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

	std::vector<GenericValue> Args(0); // Empty vector as no args are passed
	GenericValue value = engine->runFunction(ForLoopFnc, Args);

	outs() << "Return value = " << value.IntVal << "\n";


	return 0;
}

Function* GenForLoop(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters)
{
	Function *ForLoopFnc = 
		cast<Function>( module->getOrInsertFunction("ForLoopFnc", Type::getInt32Ty(context)) );

	Value* N = ConstantInt::get(builder.getInt32Ty(), iters);
	Value* zero = ConstantInt::get(builder.getInt32Ty(), 0);
	Value* one = ConstantInt::get(builder.getInt32Ty(), 1);
	Value* two = ConstantInt::get(builder.getInt32Ty(), 2);
	Value* three = ConstantInt::get(builder.getInt32Ty(), 3);

	/// BBs Outline
	BasicBlock *EntryBB = BasicBlock::Create(context, "entry", ForLoopFnc);
	BasicBlock *ForLoop1EntryBB = BasicBlock::Create(context, "forLoop1Entry", ForLoopFnc);
	BasicBlock *ForLoop1BodyBB = BasicBlock::Create(context, "forLoop1Body", ForLoopFnc);
		BasicBlock *ForLoop2EntryBB = BasicBlock::Create(context, "forLoop2Entry", ForLoopFnc);
		BasicBlock *ForLoop2BodyBB = BasicBlock::Create(context, "forLoop2Body", ForLoopFnc);
			BasicBlock *IfEntryBB = BasicBlock::Create(context, "ifEntry", ForLoopFnc);
			BasicBlock *ThenBB = BasicBlock::Create(context, "then", ForLoopFnc);
			BasicBlock *ElseBB = BasicBlock::Create(context, "else", ForLoopFnc);
		BasicBlock *ForLoop2ExitBB = BasicBlock::Create(context, "forLoop2Exit", ForLoopFnc);
	BasicBlock *ForLoop1ExitBB = BasicBlock::Create(context, "forLoop1Exit", ForLoopFnc);
	BasicBlock *ExitBB = BasicBlock::Create(context, "exit", ForLoopFnc);
	
	/// Variables
	Value *ifiLTN, *ifjLTN, *ifEqual, *i, *j, *iVal, *jVal, 
		  *counter, *counterVal, *cmpVal, *returnValue;


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
		jVal = builder.CreateAdd(jVal, one);
		builder.CreateStore(jVal, j);
		builder.CreateBr(IfEntryBB);

			/// IfEntryBB
			builder.SetInsertPoint(IfEntryBB);
			counterVal = builder.CreateLoad(counter, "counterVal");
			cmpVal = builder.CreateAdd(builder.CreateMul(counterVal, two), one);
			cmpVal = builder.CreateURem(cmpVal, three, "cmpVal");
			ifEqual = builder.CreateICmpEQ(cmpVal, zero, "ifEqual");
			builder.CreateCondBr(ifEqual, ThenBB, ElseBB);

			/// ThenBB
			builder.SetInsertPoint(ThenBB);
			counterVal = builder.CreateAdd(counterVal, one);
			builder.CreateStore(counterVal, counter);
			builder.CreateBr(ForLoop2EntryBB);

			/// ElseBB
			builder.SetInsertPoint(ElseBB);
			builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "dummyAlloca");
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

void print(Analysis_t &analysis)
{
	outs() << "----------------------------------------\n";
	outs() << "Loop Analysis Results (main):\n";
	for (Analysis_t::iterator i=analysis.begin(); i != analysis.end(); ++i)
	{
		i->second->printAnalysis();
	}
	outs() << "----------------------------------------\n";

}


