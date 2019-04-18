#include <LLVMHeaders.h>

/**
 * @brief      Generates a test set of llvm ir code (Test1)
 *
 * @param      context  The llvm context
 * @param      builder  The llvm builder to assist in the ir code gen
 * @param      module   The llvm module 
 * @param[in]  iters    The number of iterations for the for loop entry
 *
 * @return     { A set of ir code that creates a for loop with the following lines:
 * 					 d = x * y + a + 3
 * 					 f = 3 * a
 * 					 c = d + f}
 */
llvm::Function* generateTest1(llvm::LLVMContext &context, llvm::IRBuilder<> &builder, llvm::Module* module, int iters)
{
  	llvm::Function *TestFnc =
    	llvm::cast<llvm::Function>( module->getOrInsertFunction("Callee", llvm::Type::getVoidTy(context)) );

  	llvm::Value* N = llvm::ConstantInt::get(builder.getInt32Ty(), iters);
  	llvm::Value* zero = llvm::ConstantInt::get(builder.getInt32Ty(), 0);
  	llvm::Value* one = llvm::ConstantInt::get(builder.getInt32Ty(), 1);
  	llvm::Value* two = llvm::ConstantInt::get(builder.getInt32Ty(), 2);
  	llvm::Value* three = llvm::ConstantInt::get(builder.getInt32Ty(), 3);

	/// BBs Outline
  	llvm::BasicBlock *EntryBB = llvm::BasicBlock::Create(context, "entry", TestFnc);
  	llvm::BasicBlock *ForEntryBB = llvm::BasicBlock::Create(context, "forEntry", TestFnc);
  	llvm::BasicBlock *ForBodyBB = llvm::BasicBlock::Create(context, "forBody", TestFnc);
  	llvm::BasicBlock *ForIncBB = llvm::BasicBlock::Create(context, "forInc", TestFnc);
  	llvm::BasicBlock *ForExitBB = llvm::BasicBlock::Create(context, "forExit", TestFnc);
  	llvm::BasicBlock *ExitBB = llvm::BasicBlock::Create(context, "exit", TestFnc);

	/// Variables
  	llvm::Value *if_i_LT_N, *a, *c, *d, *f, *x, *y,
		*i, *aVal, *cVal, *dVal, *fVal, *xVal, *yVal, *iVal, *iValNew, *inc;

	/// EntryBB
	builder.SetInsertPoint(EntryBB);
  	a = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "a");
  	c = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "c");
  	d = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "d");
  	f = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "f");
  	x = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "x");
  	y = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "y");
  	i = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "i");
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
	xVal = builder.CreateLoad(x);
	yVal = builder.CreateLoad(y);
	aVal = builder.CreateLoad(a);
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
	iValNew = builder.CreateLoad(i);
	inc = builder.CreateAdd(iValNew, one, "i++");
	builder.CreateStore(inc, i);
	builder.CreateBr(ForEntryBB);

	/// ForExitBB
	builder.SetInsertPoint(ForExitBB);
	builder.CreateBr(ExitBB);

	/// ExitBB
	builder.SetInsertPoint(ExitBB);
  	llvm::ReturnInst::Create(context, ExitBB);

	return TestFnc;
}
