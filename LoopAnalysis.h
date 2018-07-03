#ifndef DEBUG_TYPE 
#define DEBUG_TYPE "loopAnalysis"

#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace 
{
	struct LoopAnalysis : public FunctionPass {
		int opCount = 0;
		bool opCodeDNE;
		static char ID;
		LoopAnalysis() : FunctionPass(ID) {}

		/// Used to call other passes
		void getAnalysisUsage(AnalysisUsage &AU) const
		{

		}

		virtual bool runOnFunction(Function &function)
		{
			outs() << "------------------------------------\n";
			outs() << "Loop Analysis Results:\n";
			outs() << "\tFunction = " << function.getName() << "()" << "\n";
			// Iterate over basic blocks(BB)
			for (Function::iterator BB = function.begin(), end = function.end(); BB != end; ++BB)
			{
				// Iterate over each instruction(I) in a given BB
				for (BasicBlock::iterator I = BB->begin(), end = BB->end(); I != end; ++I)
				{	
					opCount++;
				}
			}
			outs() << "\topCount = " << opCount << "\n";
			outs() << "------------------------------------\n";

			// opCount.clear();
			opCount = 0;
			return false; //all analysis passes will return false
		}

	};

} //unnamed namespace (internal linkage)
char LoopAnalysis::ID = 0;

FunctionPass *createLoopAnalysisPass() {
	return new LoopAnalysis();
}


#endif /* DEBUG_TYPE */