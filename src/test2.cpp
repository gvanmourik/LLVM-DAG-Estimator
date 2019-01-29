#include <LLVMHeaders.h>

llvm::Function* generateTest2(llvm::LLVMContext &context, llvm::IRBuilder<> &builder, llvm::Module* module,
                              int iters, llvm::Function *callee)
{
  llvm::Function *TestFnc =
    llvm::cast<llvm::Function>( module->getOrInsertFunction("Caller", llvm::Type::getVoidTy(context)) );

  llvm::Value* N = llvm::ConstantInt::get(builder.getInt32Ty(), iters);
  llvm::Value* zero = llvm::ConstantInt::get(builder.getInt32Ty(), 0);
  llvm::Value* one = llvm::ConstantInt::get(builder.getInt32Ty(), 1);

	/// BBs Outline
  llvm::BasicBlock *EntryBB = llvm::BasicBlock::Create(context, "entry", TestFnc);
  llvm::BasicBlock *ForEntryBB = llvm::BasicBlock::Create(context, "forEntry", TestFnc);
  llvm::BasicBlock *ForBodyBB = llvm::BasicBlock::Create(context, "forBody", TestFnc);
  llvm::BasicBlock *ForIncBB = llvm::BasicBlock::Create(context, "forInc", TestFnc);
  llvm::BasicBlock *ForExitBB = llvm::BasicBlock::Create(context, "forExit", TestFnc);
  llvm::BasicBlock *ExitBB = llvm::BasicBlock::Create(context, "exit", TestFnc);

	/// Variables
  llvm::Value *if_i_LT_N, *i, *iVal, *iValNew, *inc;

	/// EntryBB
	builder.SetInsertPoint(EntryBB);
  	i = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "i");
	builder.CreateStore(zero, i);
	builder.CreateBr(ForEntryBB);

	/// ForEntryBB
	builder.SetInsertPoint(ForEntryBB);
	iVal = builder.CreateLoad(i, "iVal");
	if_i_LT_N = builder.CreateICmpULT(iVal, N, "if_i_LT_N");
	builder.CreateCondBr(if_i_LT_N, ForBodyBB, ForExitBB);

	/// ForBodyBB
	builder.SetInsertPoint(ForBodyBB);
	if (callee != nullptr)
		builder.CreateCall(callee);
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

