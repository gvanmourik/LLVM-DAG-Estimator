#ifndef DEBUG_TYPE 
#define DEBUG_TYPE "loopAnalysis"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

using namespace llvm;

namespace 
{
	struct LoopAnalysis : public FunctionPass {
		// std::map<std::string, int> opCount;
		int opCount;
		bool opCodeDNE;
		static char ID;
		LoopAnalysis() : FunctionPass(ID) {}

		/// Used to call other passes
		void getAnalysisUsage(AnalysisUsage &AU) const
		{

		}

		virtual bool runOnFunction(Function &function)
		{
			outs() << "Function: " << function.getName() << "()" << "\n";
			// Iterate over basic blocks(BB)
			for (Function::iterator BB = function.begin(), end = function.end(); BB != end; ++BB)
			{
				// Iterate over each instruction(I) in a given BB
				for (BasicBlock::iterator I = BB->begin(), end = BB->end(); I != end; ++I)
				{
					// opCodeDNE = opCount.find(I->getOpcodeName()) == opCount.end();
					// if ( opCodeDNE )
					// {
					// 	opCount[I->getOpcodeName()] = 1;
					// }
					// else
					// {
					// 	opCount[I->getOpcodeName()] += 1;
					// }
					
					opCount++;
				}
			}
			outs() << "opCount = " << opCount << "\n";

			// opCount.clear();
			opCount = 0;
			return false; //all analysis passes will return false
		}

	};

} //unnamed namespace (internal linkage)
char LoopAnalysis::ID = 0;
static RegisterPass<LoopAnalysis>X("loopAnalysis", "Counts the number of operations, reads, and writes per function (should be used on IR of a for-loop)");

#endif /* DEBUG_TYPE */