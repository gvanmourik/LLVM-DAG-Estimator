#ifndef MODULE_INFO_PASS_H
#define MODULE_INFO_PASS_H

#include <unordered_map>
#include <llvm/IR/Function.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Support/raw_ostream.h>

#include "AnalysisInfo.h"
#include "FunctionInfoPass.h"
#include "DAGBuilder.h"

/// New PassManager pass
class ModuleInfoPass : public llvm::AnalysisInfoMixin<ModuleInfoPass>
{
  friend llvm::AnalysisInfoMixin<ModuleInfoPass>;
  static llvm::AnalysisKey Key;

 public:
	using Result = ModuleAnalysisInfo;

  explicit ModuleInfoPass(llvm::FunctionAnalysisManager &F) : FAM(F) {}

  ModuleAnalysisInfo run(llvm::Module &module, llvm::ModuleAnalysisManager &MAM)
	{
		ModuleAnalysisInfo analysis(&module);
		analysis.FunctionAnalyses.clear();
		gatherAnalysis(module, analysis, MAM);
		return analysis;
	}

	/// Helper function
  void gatherAnalysis(llvm::Module &module, ModuleAnalysisInfo &analysis,
                      llvm::ModuleAnalysisManager &MAM);

 private:
  llvm::FunctionAnalysisManager& FAM;

};

#endif /* MODULE_INFO_PASS_H */

