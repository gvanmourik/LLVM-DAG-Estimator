#include <LLVMHeaders.h>

/**
 * @brief      Generates a test set of llvm ir code (Test3)
 *
 * @param      context  The llvm context
 * @param      builder  The llvm builder to assist in the ir code gen
 * @param      module   The llvm module 
 * @param[in]  iters    The number of iterations for the for loop entry
 *
 * @return     { A set of ir code that creates a for loop and an inner
 * 					if statement to test the control flow of manually 
 * 					writing llvm ir.  }
 */
llvm::Function* generateTest3(llvm::LLVMContext &context, llvm::IRBuilder<> &builder, llvm::Module* module, int iters)
{
	llvm::Function *ForLoopFnc =
		llvm::cast<llvm::Function>( module->getOrInsertFunction("ForLoopFnc", llvm::Type::getInt32Ty(context)) );

  	llvm::Value* N = llvm::ConstantInt::get(builder.getInt32Ty(), iters);
  	llvm::Value* zero = llvm::ConstantInt::get(builder.getInt32Ty(), 0);
  	llvm::Value* one = llvm::ConstantInt::get(builder.getInt32Ty(), 1);
  	llvm::Value* two = llvm::ConstantInt::get(builder.getInt32Ty(), 2);
  	llvm::Value* three = llvm::ConstantInt::get(builder.getInt32Ty(), 3);

	/// BBs Outline
  	llvm::BasicBlock *EntryBB = llvm::BasicBlock::Create(context, "entry", ForLoopFnc);
  	llvm::BasicBlock *ForLoop1EntryBB = llvm::BasicBlock::Create(context, "forLoop1Entry", ForLoopFnc);
  	llvm::BasicBlock *IfEntryBB = llvm::BasicBlock::Create(context, "ifEntry", ForLoopFnc);
  	llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(context, "then", ForLoopFnc);
  	llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(context, "else", ForLoopFnc);
  	llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(context, "merge", ForLoopFnc);
  	llvm::BasicBlock *ForLoop1ExitBB = llvm::BasicBlock::Create(context, "forLoop1Exit", ForLoopFnc);
  	llvm::BasicBlock *ExitBB = llvm::BasicBlock::Create(context, "exit", ForLoopFnc);

	/// Variables
  	llvm::Value *ifiLTN, *ifEqual, *i, *iVal, *counter,
		  *counterAVal, *cmpVal, *returnValue;


	/// EntryBB
	builder.SetInsertPoint(EntryBB);
  	i = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "i");
  	counter = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "counter");
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
    	builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "dummyAlloca");
		builder.CreateBr(MergeBB);

		/// MergeBB
		builder.SetInsertPoint(MergeBB);
		iVal = builder.CreateAdd(iVal, one, "i++");
		builder.CreateStore(iVal, i);
		builder.CreateBr(ForLoop1EntryBB);

	/// ForLoop1ExitBB
	builder.SetInsertPoint(ForLoop1ExitBB);
  	builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, "dummyAlloca");
	builder.CreateBr(ExitBB);

	/// ExitBB
	builder.SetInsertPoint(ExitBB);
	returnValue = builder.CreateLoad(counter, "finalCount");
  	llvm::ReturnInst::Create(context, returnValue, ExitBB);


	return ForLoopFnc;
}

