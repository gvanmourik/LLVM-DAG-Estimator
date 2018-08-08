#ifndef FUNCTION_INFO_PASS_H
#define FUNCTION_INFO_PASS_H

#include <unordered_map>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <LLVMHeaders.h>

#include "AnalysisInfo.h"
#include "DAGBuilder.h"

/// New PassManager pass
class FunctionInfoPass : public llvm::AnalysisInfoMixin<FunctionInfoPass>
{
  friend llvm::AnalysisInfoMixin<FunctionInfoPass>;
  static llvm::AnalysisKey Key;

 public:
	using Result = FunctionAnalysisInfo;

  FunctionAnalysisInfo run(llvm::Function &function, llvm::FunctionAnalysisManager &FAM)
	{
		FunctionAnalysisInfo analysis(&function);
		gatherAnalysis(function, analysis, FAM);
		return analysis;
	}

	/// Helper function
  void gatherAnalysis(llvm::Function &function, FunctionAnalysisInfo &analysis,
                      llvm::FunctionAnalysisManager &FAM);

};

#endif /* FUNCTION_INFO_PASS_H */

