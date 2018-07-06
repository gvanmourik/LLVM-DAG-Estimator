#ifndef LOOP_INFO_ANALYSIS_H
#define LOOP_INFO_ANALYSIS_H

#include <llvm/Analysis/LoopInfo.h>

using namespace llvm;

class LoopInfoAnalysis : public LoopInfo 
{
	private:
		Loop *L;

	public:
		unsigned opCount;
		unsigned bbCount;
		unsigned readCount;
		unsigned writeCount;

		LoopInfoAnalysis(): 
			L(nullptr), opCount(0), bbCount(0), readCount(0), writeCount(0) {}
		LoopInfoAnalysis(Loop *targetLoop): 
			L(targetLoop), opCount(0), bbCount(0), readCount(0), writeCount(0) {}
		~LoopInfoAnalysis(){}

		Loop* getLoop(){ return L; }

		void setLoop(Loop *newLoop){ L=newLoop; }

		void printAnalysis()
		{
			outs() << "\t\t---> Loop Depth " << L->getLoopDepth() << " <---\n";
			outs() << "\t\topCount = " << opCount << "\n";
			outs() << "\t\tbbCount = " << bbCount << "\n";
			outs() << "\t\treadCount = " << readCount << "\n";
			outs() << "\t\twriteCount = " << writeCount << "\n";
		}

		LoopInfoAnalysis& operator=(const LoopInfoAnalysis &LA) 
		{ 
			opCount = LA.opCount;
			bbCount = LA.bbCount;
			readCount = LA.readCount;
			writeCount = LA.writeCount;

			return *this;
		}


};




#endif /* LOOP_INFO_ANALYSIS_H */