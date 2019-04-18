#ifndef DAG_BUILDER_H
#define DAG_BUILDER_H

#include <fstream>
#include <sstream>

#include "DAGNode.h"
#include "FunctionInfoPass.h"


class DAGBuilder 
{
private:
	DAGVertexList Vertices;
	DAGValueList VertexByValue;
	bool hasBeenInitialized;
	bool DAGIsLocked;


public:
	DAGBuilder() : hasBeenInitialized(false){}
	~DAGBuilder() {}

	DAGVertexList retrieveDAG() 
	{
		assert( DAGIsLocked && "DAGBuilder has not been locked! Do so before retrieving the DAG.");
		return Vertices;
	}

	void lock() { DAGIsLocked = true; }

	void init()
	{
		Vertices.clear();
		DAGIsLocked = false;
		hasBeenInitialized = true;
	}

	bool add(llvm::Value* value);

	bool addVoidFunction(llvm::Value* CallValue, int varWidth, 
						 int varDepth, int opWidth, int opDepth)
	{
		
		auto callInst = llvm::cast<llvm::CallInst>(CallValue);
		DAGNode *funcNode = addVertex(callInst, FUNC);
		auto function = callInst->getCalledFunction();
		if (function)
			funcNode->setConstName( function->getName() );
		else
			funcNode->setConstName("Indirect function call");

		//update node dimensions
		funcNode->setVarWidth(varWidth);
		funcNode->setVarDepth(varDepth);
		funcNode->setOpWidth(opWidth);
		funcNode->setOpDepth(opDepth);

		return true;
	}


private:
	bool addInstruction(llvm::Value* value, DAGNode* parentNode=nullptr);
	void addStoreInst(DAGNode* &valNode, DAGNode* &instNode, llvm::Instruction *storeInst);
	void addLoadInst(DAGNode* &valNode, DAGNode* &instNode, llvm::Instruction *loadInst);
	void checkAndSet(DAGNode* &valNode, DAGNode* &instNode, llvm::Value *value);
	void addOperand(llvm::Value* operand, DAGNode* parentNode);
	std::string getConstName(llvm::Value* constVal);
	
	DAGNode* addVertex(llvm::Value* value, vertex_t type);
	void addEdge(DAGNode* currentNode, DAGNode* nextNode);
	
	bool isValuePresent(llvm::Value* key);
	bool isInvalidOperator(llvm::Instruction *inst);
	bool isBranch(llvm::Instruction *inst);
	bool isAlloca(llvm::Instruction *inst);
	bool isReturn(llvm::Instruction *inst);

	///
	/// Traversal: 
	///  - The following functions do work, but in a limited manner
	///  - findHeight() currently only finds the variable dependence height
	///  - A section needs to be added in the findHeightWrapper() to track 
	///  	the op height
	///  - Functions also need to be added for the width
	///
	bool findSourceNodes(std::vector<DAGNode*> &sourceNodes);
	int findHeight();
	int findHeightWrapper(DAGNode *node);


public:
	void print();
	bool DOTGenerateFile(std::string functionName);
	bool DOTAddSuccessor(std::ostringstream &outSS, DAGNode* parentNode);

};


#endif /* DAG_BUILDER_H */
