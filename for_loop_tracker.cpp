#include <llvm/ADT/APInt.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/CodeGen/SelectionDAG.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
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
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/CFGPrinter.h>
#include <llvm/Analysis/InstructionSimplify.h>
#include <llvm/Analysis/OptimizationRemarkEmitter.h>
#include <llvm/Target/TargetMachine.h>
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
// #include "ModuleInfoPass.h"
// #include "LoopInfoAnalysisPass.h"
#include "FunctionInfoPass.h"


using namespace llvm;

/// Function declarations
Function* generateForLoop(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters);
PassBuilder::OptimizationLevel setOptLevel(std::string &optLevelString);
ExecutionEngine *buildExecutionEngine(std::unique_ptr<Module> &module);
static TargetMachine *buildTargetMachine();
#ifdef GEN_CFG
	void generateCFG(Function *targetFnc, std::string cfg_title, PassBuilder &passBuilder, bool debug);
#endif

// TEST
Function* generateTest1(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters);
Function* generateTest2(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters, Function *callee);


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
	static LLVMContext context;
	static IRBuilder<> builder(context);
	std::unique_ptr<Module> mainModule( new Module("ForLoopFncModule", context) );
	Module *module = mainModule.get();
	PassBuilder passBuilder;
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();

	/// Specify target features
	static auto targetMachine = buildTargetMachine();
	module->setDataLayout(targetMachine->createDataLayout());
	module->setTargetTriple( targetMachine->getTargetTriple().getTriple() );

	/// Returned IR function
	// static Function *ForLoopFnc = generateForLoop( context, builder, module, N );
	// static Function *TestFnc = generateTest( context, builder, module, N );
	// generateForLoop(context, builder, module, N);
	static Function *callee = generateTest1(context, builder, module, N);
	static Function *caller = generateTest2(context, builder, module, N, callee);

	/// CFG Before
	#ifdef GEN_CFG
		generateCFG(TestFnc, "TestFnc", passBuilder, false);
		generateCFG(ForLoopFnc, "ForLoopFncBefore", passBuilder, false);
	#endif

	/// Function analysis and pass managers
	bool DebugPM = false;
	bool DebugAM = false;
	static FunctionPassManager *FPM = 
		new FunctionPassManager(DebugPM);

	static ModuleAnalysisManager *MAM = 
		new ModuleAnalysisManager(DebugAM);
	// static CGSCCAnalysisManager *CGAM = 
	// 	new CGSCCAnalysisManager(DebugAM);
	static FunctionAnalysisManager *FAM = 
		new FunctionAnalysisManager(DebugAM);
	static LoopAnalysisManager *LAM = 
		new LoopAnalysisManager(DebugAM);

	// *FPM = passBuilder.buildFunctionSimplificationPipeline(
	// 	optLevel, 
	// 	PassBuilder::ThinLTOPhase::None, 
	// 	DebugPM);

	/// Manually register the proxies used in the pipeline
	// FAM->registerPass([&]{ return LoopAnalysisManagerFunctionProxy(*LAM); });
	// LAM->registerPass([&]{ return FunctionAnalysisManagerLoopProxy(*FAM); });
	// passBuilder.crossRegisterProxies(*LAM, *FAM, *CGAM, *MAM);

	/// Use the below method to register all added transform passes rather than 
	/// passing in each individual pass as a lambda to register that pass.
	/// [ example: FAM->registerPass([&]{ return SROA(); }); ]
	// passBuilder.registerFunctionAnalyses(*FAM);
	// passBuilder.registerLoopAnalyses(*LAM);

	
	/// Custom analysis passes
	// FAM->registerPass([&]{ return LoopInfoAnalysisPass(); });
	FAM->registerPass([&]{ return FunctionInfoPass(); });
	// MAM->registerPass([&]{ return ModuleInfoPass(*FAM); });

	// passBuilder.registerModuleAnalyses(*MAM);
	passBuilder.registerFunctionAnalyses(*FAM);
	passBuilder.registerLoopAnalyses(*LAM);
	/// Collect and print result
	// auto &result = FAM->getResult<LoopInfoAnalysisPass>(*ForLoopFnc);
	// print(result); //prints in reverse order


	/// Transform passes	
	// outs() << "--------------------BEFORE-----------------------\n" << *module << "\n"; //print before
	// FPM->run(*ForLoopFnc, *FAM);
	// outs() << "---------------------AFTER-----------------------\n" << *module << "\n"; //print after
	/// CFG After
	#ifdef GEN_CFG
		// generateCFG(ForLoopFnc, "ForLoopFncAfter", passBuilder, false);
	#endif


	auto engine = buildExecutionEngine(mainModule);
	std::vector<GenericValue> Args(0); // Empty vector as no args are passed
	// GenericValue value = engine->runFunction(ForLoopFnc, Args);


	// outs() << *caller;
	auto &FA2 = FAM->getResult<FunctionInfoPass>(*caller);
	outs() << "----------------------------------------\n";
	FA2.printAnalysis();
	outs() << "----------------------------------------\n";
	// auto &result = MAM->getResult<ModuleInfoPass>(*module);
	// print<ModuleAnalysisInfo>(result);


	// outs() << "TestFnc() return value = " << value.IntVal << "\n";
	outs() << "Opt Level: " << optLevelString << "\n";
	// outs() << "--->SEE ABOVE for LoopInfoAnalysisPass Results<---\n";


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
	Value *ifiLTN, *ifEqual, *i, *iVal, *counter, 
		  *counterAVal, *cmpVal, *returnValue;


	/// EntryBB
	builder.SetInsertPoint(EntryBB);
	i = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "i");
	counter = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "counter");	
	builder.CreateStore(zero, i);
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

Function* generateTest1(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters)
{
	Function *TestFnc = 
		cast<Function>( module->getOrInsertFunction("Callee", Type::getVoidTy(context)) );

	Value* N = ConstantInt::get(builder.getInt32Ty(), iters);
	Value* zero = ConstantInt::get(builder.getInt32Ty(), 0);
	Value* one = ConstantInt::get(builder.getInt32Ty(), 1);
	Value* two = ConstantInt::get(builder.getInt32Ty(), 2);
	Value* three = ConstantInt::get(builder.getInt32Ty(), 3);

	/// BBs Outline
	BasicBlock *EntryBB = BasicBlock::Create(context, "entry", TestFnc);
	BasicBlock *ForEntryBB = BasicBlock::Create(context, "forEntry", TestFnc);
	BasicBlock *ForBodyBB = BasicBlock::Create(context, "forBody", TestFnc);
	BasicBlock *ForIncBB = BasicBlock::Create(context, "forInc", TestFnc);
	BasicBlock *ForExitBB = BasicBlock::Create(context, "forExit", TestFnc);
	BasicBlock *ExitBB = BasicBlock::Create(context, "exit", TestFnc);
	
	/// Variables
	Value *if_i_LT_N, *a, *c, *d, *f, *x, *y, 
		  *i, *aVal, *cVal, *dVal, *fVal, *xVal, *yVal, *iVal;

	/// EntryBB
	builder.SetInsertPoint(EntryBB);
	a = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "a");
	c = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "c");
	d = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "d");
	f = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "f");
	x = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "x");
	y = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "y");
	i = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "i");
	builder.CreateStore(two, a);
	builder.CreateStore(two, x);
	builder.CreateStore(three, y);
	builder.CreateStore(zero, i);
	builder.CreateBr(ForEntryBB);

	/// ForEntryBB
	builder.SetInsertPoint(ForEntryBB);
	iVal = builder.CreateLoad(i, "iVal");
	if_i_LT_N = builder.CreateICmpULT(iVal, N, "if_i_LT_N");
	builder.CreateCondBr(if_i_LT_N, ForBodyBB, ForExitBB);

	/// ForBodyBB
	builder.SetInsertPoint(ForBodyBB);
	// d = x * y + a + 3
	xVal = builder.CreateLoad(x, "xVal");
	yVal = builder.CreateLoad(y, "yVal");
	aVal = builder.CreateLoad(a, "aVal");
	dVal = builder.CreateMul(xVal, yVal, "x*y");
	aVal = builder.CreateAdd(aVal, three, "a+3");
	dVal = builder.CreateAdd(dVal, aVal, "x*y+a+3");
	builder.CreateStore(dVal, d);
	// f = 3 * a
	aVal = builder.CreateLoad(a, "aVal");
	fVal = builder.CreateMul(three, aVal, "3*a");
	builder.CreateStore(fVal, f);
	// c = d + f
	dVal = builder.CreateLoad(d, "dVal");
	fVal = builder.CreateLoad(f, "fVal");
	cVal = builder.CreateAdd(dVal, fVal, "d+f");
	builder.CreateStore(cVal, c);
	builder.CreateBr(ForIncBB);

	/// ForIncBB
	builder.SetInsertPoint(ForIncBB);
	iVal = builder.CreateAdd(iVal, one, "i++");
	builder.CreateStore(iVal, i);
	builder.CreateBr(ForEntryBB);
	
	/// ForExitBB
	builder.SetInsertPoint(ForExitBB);
	builder.CreateBr(ExitBB);

	/// ExitBB
	builder.SetInsertPoint(ExitBB);
	ReturnInst::Create(context, ExitBB);


	return TestFnc;
}

Function* generateTest2(LLVMContext &context, IRBuilder<> &builder, Module* module, int iters, Function *callee)
{
	Function *TestFnc = 
		cast<Function>( module->getOrInsertFunction("Caller", Type::getVoidTy(context)) );

	Value* N = ConstantInt::get(builder.getInt32Ty(), iters);
	Value* zero = ConstantInt::get(builder.getInt32Ty(), 0);
	Value* one = ConstantInt::get(builder.getInt32Ty(), 1);

	/// BBs Outline
	BasicBlock *EntryBB = BasicBlock::Create(context, "entry", TestFnc);
	BasicBlock *ForEntryBB = BasicBlock::Create(context, "forEntry", TestFnc);
	BasicBlock *ForBodyBB = BasicBlock::Create(context, "forBody", TestFnc);
	BasicBlock *ForIncBB = BasicBlock::Create(context, "forInc", TestFnc);
	BasicBlock *ForExitBB = BasicBlock::Create(context, "forExit", TestFnc);
	BasicBlock *ExitBB = BasicBlock::Create(context, "exit", TestFnc);
	
	/// Variables
	Value *if_i_LT_N, *i, *iVal;

	/// EntryBB
	builder.SetInsertPoint(EntryBB);
	i = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "i");
	builder.CreateStore(zero, i);
	builder.CreateBr(ForEntryBB);

	/// ForEntryBB
	builder.SetInsertPoint(ForEntryBB);
	iVal = builder.CreateLoad(i, "iVal");
	if_i_LT_N = builder.CreateICmpULT(iVal, N, "if_i_LT_N");
	builder.CreateCondBr(if_i_LT_N, ForBodyBB, ForExitBB);

	/// ForBodyBB
	builder.SetInsertPoint(ForBodyBB);
	builder.CreateCall(callee);
	builder.CreateBr(ForIncBB);

	/// ForIncBB
	builder.SetInsertPoint(ForIncBB);
	iVal = builder.CreateAdd(iVal, one, "i++");
	builder.CreateStore(iVal, i);
	builder.CreateBr(ForEntryBB);
	
	/// ForExitBB
	builder.SetInsertPoint(ForExitBB);
	builder.CreateBr(ExitBB);

	/// ExitBB
	builder.SetInsertPoint(ExitBB);
	ReturnInst::Create(context, ExitBB);


	return TestFnc;
}

PassBuilder::OptimizationLevel setOptLevel(std::string &optLevelString)
{
	if (optLevelString == "O1")
		return PassBuilder::O1;
	else if (optLevelString == "O2")
		return PassBuilder::O2;
	else if (optLevelString == "O3")
		return PassBuilder::O3;
	else if (optLevelString == "Os")
		return PassBuilder::Os;
	else if (optLevelString == "Oz")
		return PassBuilder::Oz;
	else
	{
		errs() << "Error: Not a prescribed optimization level. Setting to the default O1 level.\n";
		optLevelString = "O1";
		return PassBuilder::O1;
	}
}

ExecutionEngine *buildExecutionEngine(std::unique_ptr<Module> &module)
{
	std::string collectedErrors;
	ExecutionEngine *engine = 
		EngineBuilder(std::move(module))
		.setErrorStr(&collectedErrors)
		.setEngineKind(EngineKind::JIT)
		.create();
		
	if ( !engine )
	{
		std::string error = "Unable to construct execution engine: " + collectedErrors;
		perror(error.c_str());
		return nullptr;
	}
	return engine;
}

static TargetMachine *buildTargetMachine()
{
	std::string error;
	auto targetTriple = sys::getDefaultTargetTriple();
	auto target = TargetRegistry::lookupTarget(targetTriple, error);
	auto cpu = "generic";
	auto features = "";
	auto RM = Optional<Reloc::Model>();
	TargetOptions options;
	if (!target)
	{
		errs() << error;
		return nullptr;
	}

	return target->createTargetMachine(targetTriple, cpu, features, options, RM);
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

