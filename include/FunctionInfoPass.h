#ifndef FUNCTION_INFO_PASS_H
#define FUNCTION_INFO_PASS_H

#include <unordered_map>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>

#include "AnalysisInfo.h"
#include "DAGBuilder.h"

using namespace llvm;


/// New PassManager pass
class FunctionInfoPass : public AnalysisInfoMixin<FunctionInfoPass>
{
	friend AnalysisInfoMixin<FunctionInfoPass>;
	static AnalysisKey Key;

public:
	using Result = FunctionAnalysisInfo;

	// explicit FunctionInfoPass(FunctionAnalysisManager &FAM) : FAM(&FAM) {}

	FunctionAnalysisInfo run(Function &function, FunctionAnalysisManager &FAM)
	{
		FunctionAnalysisInfo analysis(&function);
		gatherAnalysis(function, analysis, FAM);
		return analysis;
	}

	/// Helper function
	void gatherAnalysis(Function &function, FunctionAnalysisInfo &analysis, FunctionAnalysisManager &FAM)
	{
		DAGBuilder *DAG_builder = new DAGBuilder();
		DAG_builder->init();

		for (auto blockIter=function.begin(); blockIter!=function.end(); ++blockIter)
		{
			BasicBlock *BB = &*blockIter;

			for (auto instIter=BB->begin(); instIter!=BB->end(); ++instIter)
			{
				Instruction *inst = &*instIter;
				analysis.instCount++;
				auto opCode = inst->getOpcode();
				DAG_builder->add(inst);

				if ( opCode == Instruction::Call )
				{
					Function *callee = cast<CallInst>(inst)->getCalledFunction();
					auto &FA = FAM.getResult<FunctionInfoPass>(*callee);
					analysis.InnerFA[callee] = &FA;
					continue;
				}

				switch ( opCode ) {
					case (Instruction::Load):
						analysis.readCount++;
						break;
					case (Instruction::Store):
						analysis.writeCount++;
						break;
					default:
						break;
				}
			}
			analysis.bbCount++;
		}

		DAG_builder->lock();
		DAG_builder->fini();
		// outs() << "FunctionInfoPass...\n";
		// builder->printDependencyGraph();
		// DAG_builder->print();
		auto DG_builder = DAG_builder->getDGBuilder();
		// DG_builder->createVDG();
		// auto VDG = DG_builder->getVDG();
		// VDG.lock();
		// VDG.print();
		analysis.width = DG_builder->getVDGWidth();
		analysis.depth = DG_builder->getVDGDepth();
		// VDG.unlock();

	}

};
AnalysisKey FunctionInfoPass::Key;

/// Legacy/old PassManager pass
// class ModuleInfoWrapperPass : public ModulePass 
// {
// private:
// 	Analysis_t analysis;

// public:
// 	static char ID;

// 	ModuleInfoWrapperPass() : ModulePass(ID) {}
// 	~ModuleInfoWrapperPass(){ analysis.clear(); }

// 	/// Used to call other passes
// 	void getAnalysisUsage(AnalysisUsage &AU) const override
// 	{
// 		AU.addRequired<LoopInfoAnalysisWrapperPass>();
// 		AU.setPreservesAll();
// 	}

// 	virtual bool runOnModule(Module &module) override
// 	{
// 		calculateAnalysis(module);
// 		return false; //all analysis passes will return false
// 	}

// 	/// Clear between runs
// 	// void releaseMemory() override { analysis.clear(); }

// 	/// Helper function
// 	void calculateAnalysis(Module &module)
// 	{
// 		auto &LA = getAnalysis<LoopInfoAnalysisWrapperPass>().getLoopAnalysis();

// 		/// Use LoopInfo to iterate over each loop and sub-loop
// 		for (LoopInfo::iterator iterL=LI.begin(); iterL!=LI.end(); ++iterL)
// 		{
// 			analysis[*iterL] = new LoopInfoAnalysis(*iterL);
// 			emitLoopInfo(*iterL);
// 		}
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
// 						analysis[L]->readCount++;
// 						break;
// 					case (Instruction::Store):
// 						analysis[L]->writeCount++;
// 						break;
// 					default:
// 						break;
// 				}
// 				analysis[L]->opCount++;
// 			}
// 			analysis[L]->bbCount++;
// 		}

// 		analysis[L]->printAnalysis();

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


#endif /* FUNCTION_INFO_PASS_H */
