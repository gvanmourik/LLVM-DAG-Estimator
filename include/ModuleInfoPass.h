#ifndef MODULE_INFO_PASS_H
#define MODULE_INFO_PASS_H

#include <unordered_map>
#include <llvm/IR/Function.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Support/raw_ostream.h>

#include "AnalysisInfo.h"
#include "FunctionInfoPass.h"
#include "DAGBuilder.h"

using namespace llvm;


/// New PassManager pass
class ModuleInfoPass : public AnalysisInfoMixin<ModuleInfoPass>
{
	friend AnalysisInfoMixin<ModuleInfoPass>;
	static AnalysisKey Key;

public:
	using Result = ModuleAnalysisInfo;

	explicit ModuleInfoPass(FunctionAnalysisManager &FAM) : FAM(&FAM) {}

	ModuleAnalysisInfo run(Module &module, ModuleAnalysisManager &MAM)
	{
		ModuleAnalysisInfo analysis(&module);
		analysis.FunctionAnalyses.clear();
		gatherAnalysis(module, analysis, MAM);
		return analysis;
	}

	/// Helper function
	void gatherAnalysis(Module &module, ModuleAnalysisInfo &analysis, ModuleAnalysisManager &MAM)
	{

		/// iterate through each function
				// collect loop information (LoopInfoAnalysisPass)
			/// iterate through each block
			/// iterate through each instruction
				// build dag for entire function
				// build variable graph for function
			/// MERGE loop and function analysis


		DAGBuilder *DAG_builder = new DAGBuilder();
		DAG_builder->init();

		for (auto fncIter=module.begin(); fncIter!=module.end(); ++fncIter)
		{
			Function *function = &*fncIter;
			auto &FA = FAM->getResult<FunctionInfoPass>(*function);
			analysis.FunctionAnalyses[function] = &FA;

			for (auto blockIter=function->begin(); blockIter!=function->end(); ++blockIter)
			{
				BasicBlock *BB = &*blockIter;

				for (auto instIter=BB->begin(); instIter!=BB->end(); ++instIter)
				{
					Instruction *inst = &*instIter;
					DAG_builder->add(inst);

					switch ( inst->getOpcode() ) {
					case (Instruction::Load):
						analysis.readCount++;
						break;
					case (Instruction::Store):
						analysis.writeCount++;
						break;
					default:
						break;
					}

					analysis.instCount++;
				}
				analysis.bbCount++;
			}
		}

		DAG_builder->lock();
		DAG_builder->fini();
		// DAG_builder->print();
		auto DG_builder = DAG_builder->getDGBuilder();
		analysis.varWidth = DG_builder->getVDGWidth();
		analysis.varDepth = DG_builder->getVDGDepth();
		analysis.opWidth = DG_builder->getODGWidth();
		analysis.opDepth = DG_builder->getODGDepth();
	
	}

private:
	FunctionAnalysisManager *FAM;


};
AnalysisKey ModuleInfoPass::Key;

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


#endif /* MODULE_INFO_PASS_H */