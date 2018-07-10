#ifndef LOOP_INFO_ANALYSIS_PASS_H
#define LOOP_INFO_ANALYSIS_PASS_H

#include <map>
#include <llvm/IR/Function.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/Analysis/LoopInfoAnalysis.h>

using namespace llvm;

/// Helpful typedef
typedef std::map<Loop*, LoopInfoAnalysis*> Analysis_t;


class LoopInfoAnalysisPass : public AnalysisInfoMixin<LoopInfoAnalysisPass>
{
	friend AnalysisInfoMixin<LoopInfoAnalysisPass>;
	static AnalysisKey Key;

// private:
// 	Analysis_t analysis;

public:
	using Result = Analysis_t;

	Analysis_t run(Function &function, FunctionAnalysisManager &FAM)
	{
		analysis.clear();
		gatherAnalysis(function, analysis, FAM);
		return analysis;
	}

	/// Helper function
	void gatherAnalysis(Function &function, Analysis_t &analysis, FunctionAnalysisManager &FAM)
	{
		outs() << "----------------------------------------\n";
		outs() << "Loop Analysis Results (pass):\n";
		outs() << "\tFunction = " << function.getName() << "()" << "\n";

		LoopInfo &LI = FAM.getResult<LoopAnalysis>(function);

		/// Use LoopInfo to iterate over each loop and sub-loop
		for (LoopInfo::iterator iterL=LI.begin(), endLI=LI.end(); iterL != endLI; ++iterL)
		{
			analysis[*iterL] = new LoopInfoAnalysis(*iterL);
			emitLoopInfo(*iterL, analysis);
		}
		outs() << "----------------------------------------\n";
	}

	/// Supports nested loops
	void emitLoopInfo(Loop *L, Analysis_t &analysis)
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
			emitLoopInfo(*iterSL, analysis);
		}
	}

private:
	Analysis_t analysis;


};

AnalysisKey LoopInfoAnalysisPass::Key;


#endif /* LOOP_INFO_ANALYSIS_PASS_H */