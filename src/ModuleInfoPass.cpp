#include "ModuleInfoPass.h"

llvm::AnalysisKey ModuleInfoPass::Key;

/// Helper function
void 
ModuleInfoPass::gatherAnalysis(llvm::Module &module, ModuleAnalysisInfo &analysis,
                                   llvm::ModuleAnalysisManager &MAM)
{

  /// iterate through each function
      // collect loop information (LoopInfoAnalysisPass)
    /// iterate through each block
    /// iterate through each instruction
      // build dag for entire function
      // build variable graph for function
    /// MERGE loop and function analysis


  DAGBuilder *builder = new DAGBuilder();
  builder->init();

  for (auto fncIter=module.begin(); fncIter!=module.end(); ++fncIter)
  {
    llvm::Function *function = &*fncIter;
    auto &FA = FAM.getResult<FunctionInfoPass>(*function);
    analysis.FunctionAnalyses[function] = &FA;

    for (auto blockIter=function->begin(); blockIter!=function->end(); ++blockIter)
    {
      llvm::BasicBlock *BB = &*blockIter;

      for (auto instIter=BB->begin(); instIter!=BB->end(); ++instIter)
      {
        llvm::Instruction *inst = &*instIter;
        builder->add(inst);

        switch ( inst->getOpcode() ) {
        case (llvm::Instruction::Load):
          analysis.readCount++;
          break;
        case (llvm::Instruction::Store):
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

  builder->lock();
  // builder->fini();
  llvm::outs() << "ModuleInfoPass...\n";
  // builder->print();
  // builder->printDependencyGraph();
  //analysis.width = builder->getVarWidth();
  //analysis.depth = builder->getVarDepth();
}
