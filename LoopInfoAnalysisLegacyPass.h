#ifndef LOOP_INFO_ANALYSIS_LEGACY_PASS_H
#define LOOP_INFO_ANALYSIS_LEGACY_PASS_H
#ifndef DEBUG_TYPE
#define DEBUG_TYPE "LoopInfoAnalysisLegacyPass"

#include <map>
#include <llvm/IR/Function.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Support/raw_ostream.h>

#include "LoopInfoAnalysis.h"

using namespace llvm;

/// Helpful typedef
typedef std::map<Loop*, LoopInfoAnalysis*> Analysis_t;

/// Functions
void emitLoopInfo(Loop *L);

namespace 
{
	class LoopInfoAnalysisLegacyPass : public FunctionPass 
	{
	private:
		Analysis_t analysis;

	public:
		static char ID;

		LoopInfoAnalysisLegacyPass() : FunctionPass(ID) {}
		~LoopInfoAnalysisLegacyPass(){ analysis.clear(); }

		/// Used to call other passes
		void getAnalysisUsage(AnalysisUsage &AU) const override
		{
			AU.addRequired<LoopInfoWrapperPass>();
			AU.setPreservesAll();
		}

		virtual bool runOnFunction(Function &function) override
		{
			calculateAnalysis(function);
			return false; //all analysis passes will return false
		}

		/// Clear between runs
		void releaseMemory() override { analysis.clear(); }

		/// Helper function
		void calculateAnalysis(Function &function)
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

	};

}

/// typeid of pass
char LoopInfoAnalysisLegacyPass::ID = 0;

/// Required in order to use with the old FPM
FunctionPass *createLoopInfoAnalysisLegacyPass() {
	return new LoopInfoAnalysisLegacyPass();
}


#endif /* DEBUG_TYPE */
#endif /* LOOP_INFO_ANALYSIS_LEGACY_PASS_H */
