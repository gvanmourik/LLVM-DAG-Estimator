#ifndef DEBUG_TYPE 
#define DEBUG_TYPE "loopAnalysisPass"

// #include <llvm/Pass.h>
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

			/// Use the loop info to iterate over each loop
			for (LoopInfo::iterator iterL=LI.begin(), endLI=LI.end(); iterL != endLI; ++iterL)
			{
				emitLoopInfo(*iterL);
			}
			// outs() << "\t\tLoop Depth = " << LI.getLoopDepth() << "\n";
			outs() << "------------------------------------\n";

			// opCount.clear();
			opCount = 0;
			return false; //all analysis passes will return false
		}

	};

} //unnamed namespace (internal linkage)
char LoopAnalysisPass::ID = 0;

FunctionPass *createLoopAnalysisPass() {
	return new LoopAnalysisPass();
}

/// Supports nested loops
void emitLoopInfo(Loop *L)
{
	unsigned opCount = 0;
	unsigned bbCount = 0;
	for (Loop::block_iterator iterB=L->block_begin(); iterB != L->block_end(); ++iterB)
	{
		BasicBlock *BB = *iterB;
		/// Iterate over each instruction(I) in a given BB
		for (BasicBlock::iterator iterI = BB->begin(), end = BB->end(); iterI != end; ++iterI)
		{	
			opCount++;
		}
		bbCount++;
	}
	/// Output opCount for a given loop 
	outs() << "\t\topCount = " << opCount << " (Loop depth " << L->getLoopDepth() << ")\n";
	outs() << "\t\tbbCount = " << bbCount << " (Loop depth " << L->getLoopDepth() << ")\n";

	std::vector<Loop*> subLoops = L->getSubLoops();
	for (Loop::iterator iterSL=subLoops.begin(), lastSL=subLoops.end(); iterSL != lastSL; ++iterSL)
	{
		emitLoopInfo(*iterSL);
	}
}


#endif /* DEBUG_TYPE */