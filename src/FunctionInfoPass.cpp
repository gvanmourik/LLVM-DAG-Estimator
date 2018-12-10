#include "FunctionInfoPass.h"

llvm::AnalysisKey FunctionInfoPass::Key;

/// Helper function
void 
FunctionInfoPass::gatherAnalysis(llvm::Function &function, FunctionAnalysisInfo &analysis,
                                 llvm::FunctionAnalysisManager &FAM)
{
  DAGBuilder *DAG_builder = new DAGBuilder();
  DAG_builder->init();

  for (auto blockIter=function.begin(); blockIter!=function.end(); ++blockIter){
    llvm::BasicBlock *BB = &*blockIter;
    for (auto instIter=BB->begin(); instIter!=BB->end(); ++instIter) {
      llvm::Instruction *inst = &*instIter;
      analysis.instCount++;
      auto opCode = inst->getOpcode();

      if ( opCode == llvm::Instruction::Call )
      {
        llvm::Function *callee = llvm::cast<llvm::CallInst>(inst)->getCalledFunction();
        auto &FA = FAM.getResult<FunctionInfoPass>(*callee);
        analysis.InnerFA[callee] = &FA;
        continue;
      }
      else 
      {
        DAG_builder->add(inst);
      }

      switch ( opCode ) {
        case (llvm::Instruction::Load):
          analysis.readCount++;
          break;
        case (llvm::Instruction::Store):
          analysis.writeCount++;
          break;
        default:
          break;
      }
    }
    analysis.bbCount++;
  }

  DAG_builder->lock();
  // DAG_builder->fini();
  DAG_builder->print();
  // auto DG_builder = DAG_builder->getDGBuilder();
  // analysis.varWidth = DG_builder->getVDGWidth();
  // analysis.varDepth = DG_builder->getVDGDepth();
  // analysis.opWidth = DG_builder->getODGWidth();
  // analysis.opDepth = DG_builder->getODGDepth();

}

