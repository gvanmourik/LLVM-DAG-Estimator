#ifndef LOOP_INFO_ANALYSIS_PASS_H
#define LOOP_INFO_ANALYSIS_PASS_H
#ifndef DEBUG_TYPE
#define DEBUG_TYPE "LoopInfoAnalysisPass"

#include <map>
#include <llvm/IR/Function.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Support/raw_ostream.h>

#include "LoopInfoAnalysis.h"

using namespace llvm;


/// Global Variables
std::map<Loop*, LoopInfoAnalysis*> analysis;

/// Functions
void emitLoopInfo(Loop *L);

namespace 
{
	struct LoopInfoAnalysisPass : public FunctionPass {
		int opCount = 0;
		bool opCodeDNE;
		static char ID;
		LoopInfoAnalysisPass() : FunctionPass(ID) {}

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
				analysis[*iterL] = new LoopInfoAnalysis(*iterL);
				emitLoopInfo(*iterL);
			}
			outs() << "------------------------------------\n";

			opCount = 0;
			return false; //all analysis passes will return false
		}

	};

}

/// typeid of pass
char LoopInfoAnalysisPass::ID = 0;

/// Required in order to use with the FPM
FunctionPass *createLoopInfoAnalysisPass() {
	return new LoopInfoAnalysisPass();
}

/// Supports nested loops
void emitLoopInfo(Loop *L)
{

	for (Loop::block_iterator iterB=L->block_begin(); iterB != L->block_end(); ++iterB)
	{
		BasicBlock *BB = *iterB;
		/// Iterate over each instruction(I) in a given BB
		for (BasicBlock::iterator iterI = BB->begin(), end = BB->end(); iterI != end; ++iterI)
		{	
			Instruction *I = &*iterI;
			switch ( I->getOpcode() ) {
				case (Instruction::Load):
					analysis[L]->readCount++;
					break;
				case (Instruction::Store):
					analysis[L]->writeCount++;
					break;
				default:
					break;
			}
			analysis[L]->opCount++;
		}
		analysis[L]->bbCount++;
	}

	analysis[L]->printAnalysis();

	std::vector<Loop*> subLoops = L->getSubLoops();
	for (Loop::iterator iterSL=subLoops.begin(), lastSL=subLoops.end(); iterSL != lastSL; ++iterSL)
	{
		analysis[*iterSL] = new LoopInfoAnalysis(*iterSL);
		emitLoopInfo(*iterSL);
	}
}


#endif /* DEBUG_TYPE */
#endif /* LOOP_INFO_ANALYSIS_PASS_H */
