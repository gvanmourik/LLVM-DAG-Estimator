#include "FunctionInfoPass.h"

llvm::AnalysisKey FunctionInfoPass::Key;

/// Helper function
void 
FunctionInfoPass::gatherAnalysis(llvm::Function &function, FunctionAnalysisInfo &analysis,
                                 llvm::FunctionAnalysisManager &FAM)
{
  DAGBuilder *builder = new DAGBuilder();
  builder->init();

  for (auto blockIter=function.begin(); blockIter!=function.end(); ++blockIter){
    llvm::BasicBlock *BB = &*blockIter;
    for (auto instIter=BB->begin(); instIter!=BB->end(); ++instIter) {
      llvm::Instruction *inst = &*instIter;
      analysis.instCount++;
      auto opCode = inst->getOpcode();
      builder->add(inst);

      if ( opCode == llvm::Instruction::Call )
      {
        llvm::Function *callee = llvm::cast<llvm::CallInst>(inst)->getCalledFunction();
        auto &FA = FAM.getResult<FunctionInfoPass>(*callee);
        analysis.InnerFA[callee] = &FA;
        continue;
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

  builder->lock();
  builder->fini();
  // builder->print();
  auto DG = builder->getDG();
  DG.lock();
  // DG.print();
  analysis.width = DG.getWidth();
  analysis.depth = DG.getDepth();
  DG.unlock();

}

