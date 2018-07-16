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

/// Comment/uncomment to disable/enable CFG generation
// #define GEN_CFG

using namespace llvm;

/// Function declarations
Function* generateForLoop(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters);
void print(Analysis_t &analysis);
#ifdef GEN_CFG
void generateCFG(Function *targetFnc, std::string cfg_title, PassBuilder &passBuilder, bool debug);
#endif


int main(int argc, char* argv[])
{
	/// User inputs
	if ( argv[1] == nullptr )
	{
		perror("Missing the optimization level argument [O1, O2, O3, Os, Oz]");
		return -1;
	}
	std::string optLevel_string = argv[1];
	PassBuilder::OptimizationLevel optLevel;
	if (optLevel_string == "O1")
		optLevel = PassBuilder::O1;
	else if (optLevel_string == "O2")
		optLevel = PassBuilder::O2;
	else if (optLevel_string == "O3")
		optLevel = PassBuilder::O3;
	else if (optLevel_string == "Os")
		optLevel = PassBuilder::Os;
	else if (optLevel_string == "Oz")
		optLevel = PassBuilder::Oz;
	else
		optLevel = PassBuilder::O1;

	int N;
	std::cout << "Enter the number of loop iterations: ";
	std::cin >> N;

	/// LLVM IR Variables
	static LLVMContext context;
	static IRBuilder<> builder(context);
	std::unique_ptr<Module> mainModule( new Module("ForLoopFncModule", context) );
	Module *module = mainModule.get();
	PassBuilder passBuilder;
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();


	/// Returned IR function
	Function *ForLoopFnc = generateForLoop( context, builder, module, N );

	/// CFG Before
	#ifdef GEN_CFG
		generateCFG(ForLoopFnc, "ForLoopFncBefore", passBuilder, false);
	#endif


	/// Function analysis and pass managers
	bool DebugPM = false;
	bool DebugAM = false;
	static FunctionPassManager *FPM = 
		new FunctionPassManager(DebugPM);
	static FunctionAnalysisManager *FAM = 
		new FunctionAnalysisManager(DebugAM);
	static LoopAnalysisManager *LAM = 
		new LoopAnalysisManager(DebugAM);

	*FPM = passBuilder.buildFunctionSimplificationPipeline(
		optLevel, 
		PassBuilder::ThinLTOPhase::None, 
		DebugPM);

	/// Manually register the proxies used in the pipeline
	FAM->registerPass([&]{ return LoopAnalysisManagerFunctionProxy(*LAM); });
	LAM->registerPass([&]{ return FunctionAnalysisManagerLoopProxy(*FAM); });
	/// Use the below method to register all added transform passes rather than 
	/// passing in each individual pass as a lambda to register that pass.
	/// [ example: FAM->registerPass([&]{ return SROA(); }); ]
	passBuilder.registerFunctionAnalyses(*FAM);
	passBuilder.registerLoopAnalyses(*LAM);

	
	/// Custom analysis pass
	FAM->registerPass([&]{ return LoopInfoAnalysisPass(); });
	/// Collect and print result
	auto &result = FAM->getResult<LoopInfoAnalysisPass>(*ForLoopFnc);
	print(result); //prints in reverse order


	/// Transform passes	
	outs() << "--------------------BEFORE-----------------------\n" << *module << "\n"; //print before
	FPM->run(*ForLoopFnc, *FAM);
	outs() << "---------------------AFTER-----------------------\n" << *module << "\n"; //print after

	/// CFG After
	#ifdef GEN_CFG
		generateCFG(ForLoopFnc, "ForLoopFncAfter", passBuilder, false);
	#endif


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
	outs() << "Opt Level: " << optLevel_string << "\n";
	outs() << "--->SEE ABOVE for LoopInfoAnalysisPass Results<---\n";


	return 0;
}

Function* generateForLoop(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters)
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
		BasicBlock *IfEntryBB = BasicBlock::Create(context, "ifEntry", ForLoopFnc);
		BasicBlock *ThenBB = BasicBlock::Create(context, "then", ForLoopFnc);
		BasicBlock *ElseBB = BasicBlock::Create(context, "else", ForLoopFnc);
		BasicBlock *MergeBB = BasicBlock::Create(context, "merge", ForLoopFnc);
	BasicBlock *ForLoop1ExitBB = BasicBlock::Create(context, "forLoop1Exit", ForLoopFnc);
	BasicBlock *ExitBB = BasicBlock::Create(context, "exit", ForLoopFnc);
	
	/// Variables
	Value *ifiLTN, *ifEqual, *i, *j, *iVal, *counter, 
		  *counterAVal, *cmpVal, *returnValue;


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
	builder.CreateCondBr(ifiLTN, IfEntryBB, ForLoop1ExitBB);

		/// IfEntryBB
		builder.SetInsertPoint(IfEntryBB);
		iVal = builder.CreateLoad(i, "iVal");
		cmpVal = builder.CreateAdd(builder.CreateMul(iVal, two), one);
		cmpVal = builder.CreateURem(cmpVal, three, "modVal");
		ifEqual = builder.CreateICmpEQ(cmpVal, zero, "ifEqual");
		builder.CreateCondBr(ifEqual, ThenBB, ElseBB);

		/// ThenBB (increment counter)
		builder.SetInsertPoint(ThenBB);
		counterAVal = builder.CreateLoad(counter, "counterAVal");
		counterAVal = builder.CreateAdd(counterAVal, one);
		builder.CreateStore(counterAVal, counter);
		builder.CreateBr(MergeBB);

		/// ElseBB (do nothing)
		builder.SetInsertPoint(ElseBB);
		///
		/// IF another option is required for the counter complete here:
		// counterBVal = builder.CreateLoad(counter, "counterBVal");
		// counterBVal = builder.CreateAdd(counterBVal, one);
		// builder.CreateStore(counterBVal, counter);
		///
		builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "dummyAlloca");
		builder.CreateBr(MergeBB);

		/// MergeBB
		builder.SetInsertPoint(MergeBB);
		iVal = builder.CreateAdd(iVal, one, "i++");
		builder.CreateStore(iVal, i);
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

#ifdef GEN_CFG
void generateCFG(Function *targetFnc, std::string cfg_title, PassBuilder &passBuilder, bool debug)
{
	targetFnc->setName(cfg_title);

	static FunctionPassManager *FPM_CFG = 
		new FunctionPassManager(debug);
	static FunctionAnalysisManager *FAM_CFG = 
		new FunctionAnalysisManager(debug);

	FPM_CFG->addPass( CFGPrinterPass() );
	passBuilder.registerFunctionAnalyses(*FAM_CFG);
	FPM_CFG->run(*targetFnc, *FAM_CFG);
}
#endif

