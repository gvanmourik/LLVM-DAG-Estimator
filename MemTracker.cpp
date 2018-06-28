#ifndef DEBUG_TYPE 
#define DEBUG_TYPE "memTracker"

#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <map>

using namespace llvm;

namespace 
{
	struct memTracker : public FunctionPass {
		// std::map<std::string, int> opCount;
		int opCount
		bool opCodeDNE;
		static char ID;
		MemTracker() : FunctionPass(ID) {}

		virtual bool runOnFunction(Function &function)
		{
			errs() << "Function: " << function.getName() << "()" << "\n";
			// Iterate over basic blocks(BB)
			for (Function::iterator BB = function.begin(), end = function.end(); BB != end; ++BB)
			{
				// Iterate over each instruction(I) in a given BB
				for (BasicBlock::iterator I = BB->begin(), end = BB->end(); I != end; ++I)
				{
					// opCodeDNE = opCount.find(I->getOpcodeName()) == opCount.end();
					// if ( opCodeDNE )
					// {
					// 	opCount[I->getOpcodeName()] = 1;
					// }
					// else
					// {
					// 	opCount[I->getOpcodeName()] += 1;
					// }
					
					opCount++;
				}
			}
			errs() << "opCount = " << opCount << "\n";

			// opCount.clear();
			opCount = 0;
			return false;
		}

	};

} //unnamed namespace (internal linkage)
char MemTracker::ID = 0;
static RegisterPass<MemTracker> X("memTracker", "Counts the number of operations, reads, and writes per function");

#endif /* DEBUG_TYPE */