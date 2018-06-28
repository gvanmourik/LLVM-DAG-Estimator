#include <llvm/ADT/APInt.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/GetElementPtrTypeIterator.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>


using namespace llvm;

// From SST-Macro
// struct Loop {
// 	struct Body {
//   		int depth;
// 	    int flops;
// 	    int intops;
// 	    int writeBytes;
// 	    int readBytes;
// 	    bool hasBranchPrediction() const 
// 	    {
// 	    	return branchPrediction.size() > 0;
// 	    }
// 	    std::string branchPrediction;
// 	    std::list<Loop> subLoops;
// 	    Body() : flops(0), intops(0), readBytes(0), writeBytes(0) {}
//   	};

//   	std::string tripCount;
//   	Body body;
//   	Loop(int depth)
//   	{
//     	body.depth = depth;
//   	}
// };


/// Function declarations
Function* InitFibonacciFnc(LLVMContext &context, IRBuilder<> &builder, Module* module, StructType *, int N);

/// Global variables
// static std::map<std::string, AllocaInst*> NamedValues;


int main(int argc, char* argv[])
{
	/// LLVM IR Variables
	static LLVMContext context;
	static IRBuilder<> builder(context);
	std::unique_ptr<Module> mainModule( new Module("structTestModule", context) );
	Module *module = mainModule.get();
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();

	/// Create LLVM Struct
	std::vector<Type*> loop_members;
	std::vector<Type*> body_members;
	StructType *LoopIR = StructType::create(context, "Loop");
	StructType *BodyIR = StructType::create(context, "Body");

	// set inner struct types
	for (int i = 0, int_loop_members = 5; i < int_loop_members; ++i)
	{
		body_members.push_back( IntegerType::get(context, sizeof(int)*8) );
	}
	BodyIR->setBody(body_members);

	// set outer struct types
	loop_members.push_back( StructType::get(context, sizeof(Loop::Body)*8) );
	LoopIR->setBody(loop_members);



	/// Returned function
	Function *StructTest = InitFibonacciFnc( context, builder, module, BodyIR, atoi(argv[1]) );

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

	// std::vector<GenericValue> Args(1); // Empty vector as no args are passed
	// GenericValue value = engine->runFunction(FibonacciFnc, Args);
	Loop::Body* function_ptr = (Loop::Body*)engine->getFunctionAddress(StructTest->getName());
	if (function_ptr == nullptr)
	{
		errs() << "Failed to collect the compiled function: " << StructTest->getName() << "\n";
		return 1;
	}

	/// Output IR module
	outs() << "\n" << *module;


	return 0;
}

Function* InitFibonacciFnc(LLVMContext &context, IRBuilder<> &builder, Module* module, StructType *BodyIR, int input)
{
	PointerType *pBodyIR = PointerType::get(BodyIR, 0);
	std::vector<Type*> args = {pBodyIR};


	// Function *StructTest = 
	// 	cast<Function>( module->getOrInsertFunction("StructTest", Type::getVoidTy(context)) );
	FunctionType *funcType = FunctionType::get(builder.getInt32Ty(), args, false);
	Function *StructTest = Function::Create(funcType, Function::ExternalLinkage, "StructTestFunc", module);

	// Value* zero = ConstantInt::get(builder.getInt32Ty(), 0);
	Value* one = ConstantInt::get(builder.getInt32Ty(), 1);
	Value* two = ConstantInt::get(builder.getInt32Ty(), 2);
	Value* N = ConstantInt::get(builder.getInt32Ty(), input);

	auto argiter = StructTest->arg_begin();
	Value *myArg = argiter;
	myArg->setName("loopBody");

	/// BBs
	BasicBlock *EntryBB = BasicBlock::Create(context, "entry", StructTest);
	BasicBlock *FirstBB = BasicBlock::Create(context, "first", StructTest);
	BasicBlock *SecondBB = BasicBlock::Create(context, "second", StructTest);
	BasicBlock *ExitBB = BasicBlock::Create(context, "exit", StructTest);
	
	/// Variables
	// Value *next, *first, *second, *count, *next1, *next2;
	// Value *flops = zero,
	// 	  *readBytes = zero, 
	// 	  *writeBytes = zero, 
	// 	  *intops = zero;
	// std::vector<Value*> body_values;
	// 	body_values.push_back(flops);
	// 	body_values.push_back(readBytes);
	// 	body_values.push_back(writeBytes);
	// 	body_values.push_back(intops);

	std::vector<Value*> offsets;
	std::vector<Value*> body_vars(BodyIR->getNumElements());
		// body_vars[0]->setName("depth");
		// body_vars[1]->setName("flops");
		// body_vars[2]->setName("intops");
		// body_vars[3]->setName("writeBytes");
		// body_vars[4]->setName("readBytes");

	APInt zero(32, 0);
	Value *ptr_index = Constant::getIntegerValue(Type::getInt32Ty(context), zero);
	Value *val_index;
	offsets.push_back(ptr_index); 	//dereference the pointer

	/// EntryBB
	builder.SetInsertPoint(EntryBB);
	for (int i = 0; i < BodyIR->getNumElements(); ++i)
	{
		APInt index(32, i);
		val_index = Constant::getIntegerValue(Type::getInt32Ty(context), index);
		offsets.push_back(val_index);	//variable offset
		body_vars[i] = builder.CreateGEP( myArg, offsets, BodyIR->getName() );
		builder.CreateStore(two, body_vars[i]);

		offsets.pop_back(); //remove last variable offset used
	}

	Value *fncReturn = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "fncReturn");
	// builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "N");
	Value *ifTwoLTN = builder.CreateICmpULT(two, N, "ifTwo_LT_N");
	builder.CreateCondBr(ifTwoLTN, FirstBB, SecondBB); //end of EntryBB

	

	/// FirstBB
	builder.SetInsertPoint(FirstBB);
	Value *body_vars_1_val = builder.CreateLoad(body_vars[1], "flopsVal");
	builder.CreateStore(body_vars_1_val, fncReturn);
	Value *fncReturnValue = builder.CreateLoad(fncReturn, "returnValue");
	Value *flopsValuePlusOne = builder.CreateAdd(fncReturnValue, one, "flopsValPlusOne");
	builder.CreateStore(flopsValuePlusOne, fncReturn);
	builder.CreateBr(ExitBB);
	/// SecondBB
	builder.SetInsertPoint(SecondBB);
	Value *body_vars_2_val = builder.CreateLoad(body_vars[2], "intopsVal");
	builder.CreateStore(body_vars_2_val, fncReturn);
	fncReturnValue = builder.CreateLoad(fncReturn, "returnValue");
	Value *intopsValuePlusOne = builder.CreateAdd(fncReturnValue, one, "intopsValPlusOne");
	builder.CreateStore(intopsValuePlusOne, fncReturn);
	builder.CreateBr(ExitBB);

	
	/// ExitBB
	builder.SetInsertPoint(ExitBB);
	// fncReturnValue = builder.CreateLoad(fncReturn, "returnValue");
	ReturnInst::Create(context, fncReturnValue, ExitBB);


	return StructTest;
}