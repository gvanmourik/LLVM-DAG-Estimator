#ifndef LOOP_INFO_ANALYSIS_PASS_H
#define LOOP_INFO_ANALYSIS_PASS_H

#include <llvm/Analysis/LoopInfo.h>
#include "FunctionInfoPass.h"

using namespace llvm;

/// New PassManager pass
class LoopInfoAnalysisPass : public AnalysisInfoMixin<LoopInfoAnalysisPass>
{
	friend AnalysisInfoMixin<LoopInfoAnalysisPass>;
	static AnalysisKey Key;

public:
	using Result = LoopAnalysis_t;

	LoopAnalysis_t run(Function &function, FunctionAnalysisManager &FAM)
	{
		gatherAnalysis(function, analysis, FAM);
		return analysis;
	}

	/// Helper function
	void gatherAnalysis(Function &function, LoopAnalysis_t &analysis, FunctionAnalysisManager &FAM)
	{
		auto &FA = FAM.getResult<FunctionInfoPass>(function);
		LoopInfo &LI = FAM.getResult<LoopAnalysis>(function);

		/// Use LoopInfo to iterate over each loop and sub-loop
		for (LoopInfo::iterator iterL=LI.begin(), endLI=LI.end(); iterL != endLI; ++iterL)
		{
			analysis[*iterL] = new LoopInfoAnalysis(*iterL, &function);
			analysis[*iterL]->ParentFA = &FA;
			emitLoopInfo(analysis[*iterL], FAM, *iterL);
		}
	}

	/// Supports nested loops
	void emitLoopInfo(LoopInfoAnalysis *currentAnalysis, FunctionAnalysisManager &FAM, Loop *L)
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
				currentAnalysis->instCount++;
				auto opCode = I->getOpcode();
				builder->add(I);

				if (opCode == Instruction::Call)
				{
					Function *callee = cast<CallInst>(I)->getCalledFunction();
					auto &FA = FAM.getResult<FunctionInfoPass>(*callee);
					currentAnalysis->InnerFA[callee] = &FA;
					continue;
				}

				switch (opCode) {
					case (Instruction::Load):
						currentAnalysis->readCount++;
						break;
					case (Instruction::Store):
						currentAnalysis->writeCount++;
						break;
					default:
						break;
				}
			}
			currentAnalysis->bbCount++;
		}

		builder->lock();
		builder->fini();
		// outs() << "LoopInfoAnalysisPass...\n";
		// builder->print();
		// builder->printDependencyGraph();
		currentAnalysis->width = builder->getVarWidth();
		currentAnalysis->depth = builder->getVarDepth();

		// analysis->SubLoops[L]->printAnalysis();

		std::vector<Loop*> subLoops = L->getSubLoops();
		for (Loop::iterator iterSL=subLoops.begin(), lastSL=subLoops.end(); iterSL != lastSL; ++iterSL)
		{
			currentAnalysis->SubLoops[*iterSL] = new LoopInfoAnalysis(*iterSL);
			emitLoopInfo(currentAnalysis->SubLoops[*iterSL], FAM, *iterSL);
		}
	}

private:
	LoopAnalysis_t analysis;


};
AnalysisKey LoopInfoAnalysisPass::Key;

/// Legacy/old PassManager pass
// class LoopInfoAnalysisWrapperPass : public FunctionPass 
// {
// private:
// 	LoopAnalysis_t analysis;

// public:
// 	static char ID;

// 	LoopInfoAnalysisWrapperPass() : FunctionPass(ID) {}
// 	~LoopInfoAnalysisWrapperPass(){ analysis.clear(); }

// 	/// Used to call other passes
// 	void getAnalysisUsage(AnalysisUsage &AU) const override
// 	{
// 		AU.addRequired<LoopInfoWrapperPass>();
// 		AU.setPreservesAll();
// 	}

// 	LoopAnalysis_t getLoopAnalysis() { return analysis; }

// 	virtual bool runOnFunction(Function &function) override
// 	{
// 		calculateAnalysis(function);
// 		return false; //all analysis passes will return false
// 	}

// 	/// Clear between runs
// 	void releaseMemory() override { analysis.clear(); }

// 	/// Helper function
// 	void calculateAnalysis(Function &function)
// 	{
// 		outs() << "------------------------------------\n";
// 		outs() << "Loop Analysis Results:\n";
// 		outs() << "\tFunction = " << function.getName() << "()" << "\n";

// 		LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

// 		/// Use LoopInfo to iterate over each loop and sub-loop
// 		for (LoopInfo::iterator iterL=LI.begin(), endLI=LI.end(); iterL != endLI; ++iterL)
// 		{
// 			analysis[*iterL] = new LoopInfoAnalysis(*iterL);
// 			emitLoopInfo(*iterL);
// 		}
// 		outs() << "------------------------------------\n";
// 	}

// 	/// Supports nested loops
// 	void emitLoopInfo(Loop *L)
// 	{
// 		for (Loop::block_iterator iterB=L->block_begin(); iterB != L->block_end(); ++iterB)
// 		{
// 			BasicBlock *BB = *iterB;
// 			/// Iterate over each instruction(I) in a given BB
// 			for (BasicBlock::iterator iterI = BB->begin(), end = BB->end(); iterI != end; ++iterI)
// 			{	
// 				Instruction *I = &*iterI;
// 				switch ( I->getOpcode() ) {
// 					case (Instruction::Load):
// 						analysis->SubLoops[L]->readCount++;
// 						break;
// 					case (Instruction::Store):
// 						analysis->SubLoops[L]->writeCount++;
// 						break;
// 					default:
// 						break;
// 				}
// 				analysis->SubLoops[L]->opCount++;
// 			}
// 			analysis->SubLoops[L]->bbCount++;
// 		}

// 		analysis->SubLoops[L]->printAnalysis();

// 		std::vector<Loop*> subLoops = L->getSubLoops();
// 		for (Loop::iterator iterSL=subLoops.begin(), lastSL=subLoops.end(); iterSL != lastSL; ++iterSL)
// 		{
// 			analysis[*iterSL] = new LoopInfoAnalysis(*iterSL);
// 			emitLoopInfo(*iterSL);
// 		}
// 	}

// };

// /// typeid of pass
// char LoopInfoAnalysisWrapperPass::ID = 0;

// /// Required in order to use with the old FPM
// FunctionPass *createLoopInfoAnalysisWrapperPass() {
// 	return new LoopInfoAnalysisWrapperPass();
// }


#endif /* LOOP_INFO_ANALYSIS_PASS_H */