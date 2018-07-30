#ifndef ANALYSIS_INFO_H
#define ANALYSIS_INFO_H

#include <llvm/Analysis/LoopInfo.h>

using namespace llvm;

class LoopInfoAnalysis;
class FunctionAnalysisInfo;
typedef std::map<Loop*, LoopInfoAnalysis*> LoopAnalysis_t;
typedef std::map<Function*, FunctionAnalysisInfo*> FunctionAnalysis_t;

class BaseAnalysisInfo
{
	public:
		unsigned instCount;
		unsigned bbCount;
		unsigned readCount;
		unsigned writeCount;
		int width;
		int depth;

		BaseAnalysisInfo() :
			instCount(0), bbCount(0), readCount(0), writeCount(0), 
			width(0), depth(0) {}
		~BaseAnalysisInfo(){}

		void printAnalysis()
		{
			outs() << "\t    instCount = " << instCount << "\n";
			outs() << "\t      bbCount = " << bbCount << "\n";
			outs() << "\t    readCount = " << readCount << "\n";
			outs() << "\t   writeCount = " << writeCount << "\n";
			outs() << "\t        Width = " << width << "\n";
			outs() << "\t        Depth = " << depth << "\n";
		}

		BaseAnalysisInfo& operator=(const BaseAnalysisInfo &FA) 
		{
			instCount = FA.instCount;
			bbCount = FA.bbCount;
			readCount = FA.readCount;
			writeCount = FA.writeCount;
			width = FA.width;
			depth = FA.depth;

			return *this;
		}


};


class LoopInfoAnalysis : public BaseAnalysisInfo 
{
	private:
		Loop *L;

	public:
		std::string functionName;

		LoopInfoAnalysis(): L(nullptr) {}
		LoopInfoAnalysis(Loop *targetLoop): L(targetLoop) {}
		~LoopInfoAnalysis(){}

		Loop* getLoop(){ return L; }

		void setLoop(Loop *newLoop) { L=newLoop; }
		void setFunctionName(std::string name) { functionName = name; }

		void printAnalysis()
		{
			outs() << "\tFunction: " << functionName << "()\n";
			outs() << "\t---> Loop Depth " << L->getLoopDepth() << " <---\n";
			BaseAnalysisInfo::printAnalysis();
		}

};


class FunctionAnalysisInfo : public BaseAnalysisInfo
{
	private:
		Function *function;

	public:
		LoopAnalysis_t LoopAnalysis;

		FunctionAnalysisInfo() {}
		FunctionAnalysisInfo(Function *function) : function(function) {}
		~FunctionAnalysisInfo(){}

		void printAnalysis()
		{
			outs() << "\tFunction: " << function->getName() << "()\n";
			BaseAnalysisInfo::printAnalysis();

			outs() << "----------------------------------------\n";
			outs() << "Loop: (FunctionAnalysisInfo)\n";
			for (auto i=LoopAnalysis.begin(); i != LoopAnalysis.end(); ++i)
			{
				i->second->printAnalysis();
			}
			outs() << "----------------------------------------\n";
		}

		FunctionAnalysisInfo& operator=(const FunctionAnalysisInfo &FA) 
		{ 
			LoopAnalysis = FA.LoopAnalysis;

			return *this;
		}

};


class ModuleAnalysisInfo : public BaseAnalysisInfo
{
	private:
		Module *module;

	public:
		FunctionAnalysis_t FunctionAnalyses;

		ModuleAnalysisInfo() {}
		ModuleAnalysisInfo(Module *module) : module(module) {}
		~ModuleAnalysisInfo(){}

		void printAnalysis()
		{
			outs() << "\tModule: " << module->getName() << "()\n";
			outs() << "\tLayout: " << module->getDataLayoutStr() << "\n";
			outs() << "\t TargetTriple = " << module->getTargetTriple() << "\n";
			outs() << "\t   SourceFile = " << module->getSourceFileName() << "\n";
			BaseAnalysisInfo::printAnalysis();

			outs() << "----------------------------------------\n";
			outs() << "Function: (ModuleAnalysisInfo)\n";
			for (auto i=FunctionAnalyses.begin(); i != FunctionAnalyses.end(); ++i)
			{
				i->second->printAnalysis();
			}
			outs() << "----------------------------------------\n";
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