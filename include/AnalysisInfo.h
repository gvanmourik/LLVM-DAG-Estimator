#ifndef ANALYSIS_INFO_H
#define ANALYSIS_INFO_H

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


class FunctionAnalysisInfo : public BaseAnalysisInfo
{
	friend LoopInfoAnalysis;

	private:
		Function *function;

	public:
		FunctionAnalysis_t InnerFA;

		FunctionAnalysisInfo() : function(nullptr) {}
		FunctionAnalysisInfo(Function *function) : function(function) {}
		~FunctionAnalysisInfo(){}

		void printAnalysis()
		{
			if (function!=nullptr)
				outs() << "\t     Function = " << function->getName() << "()\n";
			BaseAnalysisInfo::printAnalysis();

			if ( !InnerFA.empty() )
			{
				outs() << "\t----------------------------------------\n";
				outs() << "\tInnerFA: (FunctionAnalysisInfo)\n";
				for (auto i=InnerFA.begin(); i != InnerFA.end(); ++i)
				{
					i->second->printAnalysis();
				}
				outs() << "\t----------------------------------------\n";
			}
		}

		FunctionAnalysisInfo& operator=(const FunctionAnalysisInfo &FA) 
		{ 
			InnerFA.insert( FA.InnerFA.begin(), FA.InnerFA.end() );
			return *this;
		}

};


class LoopInfoAnalysis : public FunctionAnalysisInfo 
{
	private:
		Loop *L;

	public:
		FunctionAnalysisInfo *ParentFA;
		LoopAnalysis_t SubLoops;

		LoopInfoAnalysis(): L(nullptr) {}
		LoopInfoAnalysis(Loop *targetLoop): L(targetLoop) {}
		LoopInfoAnalysis(Loop *targetLoop, Function *fnc): L(targetLoop) { function = fnc; }
		~LoopInfoAnalysis(){}

		Loop* getLoop(){ return L; }

		void setLoop(Loop *newLoop) { L=newLoop; }

		void printAnalysis()
		{
			outs() << "----------------------------------------\n";
			outs() << "\t----------------------------------------\n";
			if (L!=nullptr)
			{
				outs() << "\t   Loop Depth = " << L->getLoopDepth() << "\n";
			}

			ParentFA->printAnalysis();

			if ( !SubLoops.empty() )
			{
				outs() << "\tSubLoops:\n";
				for (auto i=SubLoops.begin(); i != SubLoops.end(); ++i)
				{
					outs() << "\t     Function = " << function->getName() << "()\n";
					i->second->printAnalysis();
				}
			}
			outs() << "----------------------------------------\n";
		}

		LoopInfoAnalysis& operator=(const LoopInfoAnalysis &FA) 
		{ 
			SubLoops.insert( FA.SubLoops.begin(), FA.SubLoops.end() );
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