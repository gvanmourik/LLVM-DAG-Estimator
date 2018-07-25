#ifndef LOOP_INFO_ANALYSIS_PASS_H
#define LOOP_INFO_ANALYSIS_PASS_H

#include <unordered_map>
#include <llvm/IR/Function.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Support/raw_ostream.h>

#include "LoopInfoAnalysis.h"
#include "DAGBuilder.h"

using namespace llvm;

/// Helpful typedef
typedef std::map<Loop*, LoopInfoAnalysis*> Analysis_t;

/// New PassManager pass
class LoopInfoAnalysisPass : public AnalysisInfoMixin<LoopInfoAnalysisPass>
{
	friend AnalysisInfoMixin<LoopInfoAnalysisPass>;
	static AnalysisKey Key;

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
		// outs() << "----------------------------------------\n";
		// outs() << "Loop Analysis Results (custom pass):\n";
		// outs() << "\tFunction = " << function.getName() << "()" << "\n";
		std::string fncName = function.getName();

		LoopInfo &LI = FAM.getResult<LoopAnalysis>(function);

		/// Use LoopInfo to iterate over each loop and sub-loop
		for (LoopInfo::iterator iterL=LI.begin(), endLI=LI.end(); iterL != endLI; ++iterL)
		{
			analysis[*iterL] = new LoopInfoAnalysis(*iterL);
			analysis[*iterL]->setFunctionName(fncName);
			emitLoopInfo(*iterL, analysis);
		}
		// outs() << "----------------------------------------\n";
	}

	/// Supports nested loops
	void emitLoopInfo(Loop *L, Analysis_t &analysis)
	{
		DAGBuilder *builder = new DAGBuilder();
		builder->init();

		for (Loop::block_iterator iterB=L->block_begin(); iterB != L->block_end(); ++iterB)
		{
			BasicBlock *BB = *iterB;
			/// Iterate over each instruction(I) in a given BB
			for (BasicBlock::iterator iterI = BB->begin(), end = BB->end(); iterI != end; ++iterI)
			{	
				Instruction *I = &*iterI;

				builder->add(I);

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

		builder->lock();
		builder->fini();
		analysis[L]->width = builder->getVarWidth();
		analysis[L]->depth = builder->getVarDepth();

		// analysis[L]->printAnalysis();

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

/// Legacy/old PassManager pass
class LoopInfoAnalysisWrapperPass : public FunctionPass 
{
private:
	Analysis_t analysis;

public:
	static char ID;

	LoopInfoAnalysisWrapperPass() : FunctionPass(ID) {}
	~LoopInfoAnalysisWrapperPass(){ analysis.clear(); }

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

/// typeid of pass
char LoopInfoAnalysisWrapperPass::ID = 0;

/// Required in order to use with the old FPM
FunctionPass *createLoopInfoAnalysisWrapperPass() {
	return new LoopInfoAnalysisWrapperPass();
}


#endif /* LOOP_INFO_ANALYSIS_PASS_H */