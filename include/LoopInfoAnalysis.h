#ifndef LOOP_INFO_ANALYSIS_H
#define LOOP_INFO_ANALYSIS_H

#include <llvm/Analysis/LoopInfo.h>

using namespace llvm;

class LoopInfoAnalysis : public LoopInfo 
{
	private:
		Loop *L;

	public:
		std::string functionName;
		unsigned opCount;
		unsigned bbCount;
		unsigned readCount;
		unsigned writeCount;
		int width;
		int depth;

		LoopInfoAnalysis(): 
			L(nullptr), opCount(0), bbCount(0), readCount(0), writeCount(0), 
			width(0), depth(0) {}
		LoopInfoAnalysis(Loop *targetLoop): 
			L(targetLoop), opCount(0), bbCount(0), readCount(0), writeCount(0), 
			width(0), depth(0) {}
		~LoopInfoAnalysis(){}

		Loop* getLoop(){ return L; }

		void setLoop(Loop *newLoop) { L=newLoop; }
		void setFunctionName(std::string name) { functionName = name; }

		void printAnalysis()
		{
			outs() << "\tFunction: " << functionName << "()\n";
			outs() << "\t---> Loop Depth " << L->getLoopDepth() << " <---\n";
			outs() << "\t      opCount = " << opCount << "\n";
			outs() << "\t      bbCount = " << bbCount << "\n";
			outs() << "\t    readCount = " << readCount << "\n";
			outs() << "\t   writeCount = " << writeCount << "\n";
			outs() << "\t        Width = " << width << "\n";
			outs() << "\t        Depth = " << depth << "\n";
		}

		LoopInfoAnalysis& operator=(const LoopInfoAnalysis &LA) 
		{ 
			opCount = LA.opCount;
			bbCount = LA.bbCount;
			readCount = LA.readCount;
			writeCount = LA.writeCount;
			width = LA.width;
			depth = LA.depth;

			return *this;
		}


};




#endif /* LOOP_INFO_ANALYSIS_H */