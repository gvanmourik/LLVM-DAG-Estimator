#ifndef DEBUG_TYPE 
#define DEBUG_TYPE "loopAnalysisPass"

#include <llvm/IR/Function.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

void emitLoopInfo(Loop *L);

namespace 
{
	struct LoopAnalysisPass : public FunctionPass {
		int opCount = 0;
		bool opCodeDNE;
		static char ID;
		LoopAnalysisPass() : FunctionPass(ID) {}

		/// Used to call other passes
		void getAnalysisUsage(AnalysisUsage &AU) const
		{
			AU.addRequired<LoopInfoWrapperPass>();
			AU.setPreservesAll();
		}

		virtual bool runOnFunction(Function &function)
		{
			outs() << "------------------------------------\n";
			outs() << "Loop Analysis Results:\n";
			outs() << "\tFunction = " << function.getName() << "()" << "\n";

			LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

			/// Use LoopInfo to iterate over each loop and sub-loop
			for (LoopInfo::iterator iterL=LI.begin(), endLI=LI.end(); iterL != endLI; ++iterL)
			{
				emitLoopInfo(*iterL);
			}
			outs() << "------------------------------------\n";

			opCount = 0;
			return false; //all analysis passes will return false
		}

	};

} //unnamed namespace (internal linkage)

char LoopAnalysisPass::ID = 0;

/// Required in order to use with the FPM
FunctionPass *createLoopAnalysisPass() {
	return new LoopAnalysisPass();
}

/// Supports nested loops
void emitLoopInfo(Loop *L)
{
	unsigned opCount = 0;
	unsigned bbCount = 0;
	unsigned readCount = 0;
	unsigned writeCount = 0;

	for (Loop::block_iterator iterB=L->block_begin(); iterB != L->block_end(); ++iterB)
	{
		BasicBlock *BB = *iterB;
		/// Iterate over each instruction(I) in a given BB
		for (BasicBlock::iterator iterI = BB->begin(), end = BB->end(); iterI != end; ++iterI)
		{	
			Instruction *I = &*iterI;
			switch ( I->getOpcode() ) {
				case (Instruction::Load):
					// outs() << "recording read(" << L->getLoopDepth() << ")...\n";
					// I->dump();
					// outs() << "\n";
					readCount++;
					break;
				case (Instruction::Store):
					// outs() << "recording write(" << L->getLoopDepth() << ")...\n";
					// I->dump();
					// outs() << "\n";
					writeCount++;
					break;
				default:
					break;
			} 
			opCount++;
		}
		bbCount++;
	}

	outs() << "\t\t---> Loop Depth " << L->getLoopDepth() << " <---\n";
	outs() << "\t\topCount = " << opCount << "\n";
	outs() << "\t\tbbCount = " << bbCount << "\n";
	outs() << "\t\treadCount = " << readCount << "\n";
	outs() << "\t\twriteCount = " << writeCount << "\n";

	std::vector<Loop*> subLoops = L->getSubLoops();
	for (Loop::iterator iterSL=subLoops.begin(), lastSL=subLoops.end(); iterSL != lastSL; ++iterSL)
	{
		emitLoopInfo(*iterSL);
	}
}


#endif /* DEBUG_TYPE */