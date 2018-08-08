#ifndef ANALYSIS_INFO_H
#define ANALYSIS_INFO_H

#include <LLVMHeaders.h>

class FunctionAnalysisInfo;
typedef std::map<llvm::Function*, FunctionAnalysisInfo*> FunctionAnalysis_t;


class BaseAnalysisInfo
{
	public:
		unsigned instCount;
		unsigned bbCount;
		unsigned readCount;
		unsigned writeCount;
		int varWidth;
		int varDepth;
		int opWidth;
		int opDepth;

		BaseAnalysisInfo() :
			instCount(0), bbCount(0), readCount(0), writeCount(0), 
			varWidth(0), varDepth(0), opWidth(0), opDepth(0) {}
		~BaseAnalysisInfo(){}

		void printAnalysis()
		{
			llvm::outs() << "\t       instCount = " << instCount << "\n";
			llvm::outs() << "\t         bbCount = " << bbCount << "\n";
			llvm::outs() << "\t       readCount = " << readCount << "\n";
			llvm::outs() << "\t      writeCount = " << writeCount << "\n";
			if (varDepth != 0)
			{
				llvm::outs() << "\t  Variable Width = " << varWidth << "\n";
				llvm::outs() << "\t  Variable Depth = " << varDepth << "\n";
			}
			// if (opDepth != 0)
			// {
			// 	llvm::outs() << "\t  Operator Width = " << opWidth << "\n";
			// 	llvm::outs() << "\t  Operator Depth = " << opDepth << "\n";
			// }
		}

		BaseAnalysisInfo& operator=(const BaseAnalysisInfo &FA) 
		{
			instCount = FA.instCount;
			bbCount = FA.bbCount;
			readCount = FA.readCount;
			writeCount = FA.writeCount;
			varWidth = FA.varWidth;
			varDepth = FA.varDepth;
			opWidth = FA.opWidth;
			opDepth = FA.opDepth;

			return *this;
		}
};


class FunctionAnalysisInfo : public BaseAnalysisInfo
{
	private:
    llvm::Function *function;

	public:
		FunctionAnalysis_t InnerFA;

		FunctionAnalysisInfo() : function(nullptr) {}
    FunctionAnalysisInfo(llvm::Function *function) : function(function) {}
		~FunctionAnalysisInfo(){}

		void printAnalysis()
		{
			if (function!=nullptr)
        llvm::outs() << "\t     Function = " << function->getName() << "()\n";
			BaseAnalysisInfo::printAnalysis();

			if ( !InnerFA.empty() )
			{
        llvm::outs() << "\t----------------------------------------\n";
        llvm::outs() << "\tInnerFA: (FunctionAnalysisInfo)\n";
				for (auto i=InnerFA.begin(); i != InnerFA.end(); ++i)
				{
					i->second->printAnalysis();
				}
        llvm::outs() << "\t----------------------------------------\n";
			}
		}

		FunctionAnalysisInfo& operator=(const FunctionAnalysisInfo &FA) 
		{ 
			InnerFA.insert( FA.InnerFA.begin(), FA.InnerFA.end() );
			return *this;
		}

};


class ModuleAnalysisInfo : public BaseAnalysisInfo
{
	private:
    llvm::Module *module;

	public:
		FunctionAnalysis_t FunctionAnalyses;

		ModuleAnalysisInfo() {}
    	ModuleAnalysisInfo(llvm::Module *module) : module(module) {}
		~ModuleAnalysisInfo(){}

		void printAnalysis()
		{
			llvm::outs() << "----------------------------------------\n";
			llvm::outs() << "Module: " << module->getName() << "\n";
			llvm::outs() << "\t          Layout = " << module->getDataLayoutStr() << "\n";
			llvm::outs() << "\t    TargetTriple = " << module->getTargetTriple() << "\n";
			llvm::outs() << "\t      SourceFile = " << module->getSourceFileName() << "\n";
			BaseAnalysisInfo::printAnalysis();

      		llvm::outs() << "----------------------------------------\n";
      		llvm::outs() << "Function: (ModuleAnalysisInfo)\n";
			for (auto i=FunctionAnalyses.begin(); i != FunctionAnalyses.end(); ++i)
			{
				i->second->printAnalysis();
			}
      		llvm::outs() << "----------------------------------------\n";
		}

		ModuleAnalysisInfo& operator=(ModuleAnalysisInfo &MA) 
		{ 
			auto to=FunctionAnalyses.begin();
			for (auto from=MA.FunctionAnalyses.begin(); from!=MA.FunctionAnalyses.end(); ++from)
			{
				if ( to!=FunctionAnalyses.end() )
					FunctionAnalyses[to->first] = MA.FunctionAnalyses[from->first];
				to++;
			}

			return *this;
		}
};


#endif /* ANALYSIS_INFO_H */
