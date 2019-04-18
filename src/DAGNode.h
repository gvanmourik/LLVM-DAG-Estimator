#ifndef DAG_NODE_H
#define DAG_NODE_H

#include <cstdint>
#include <sstream>
#include <map>
#include <llvm/IR/Instruction.h>

#include "LLVMHeaders.h"

typedef enum vertex_t{VAL, INST, FUNC} vertex_t;

class DAGNode;
typedef std::map<DAGNode*, DAGNode*> DAGVertexList;
typedef std::map<llvm::Value*, DAGNode*> DAGValueList;

class DAGNode {

private:
	int varWidth;
	int varDepth;
	int opWidth;
	int opDepth;
	bool visited;
	DAGNode* valueNode;
	llvm::Value* llvmValue;
	vertex_t type;
	std::string constName;
	DAGVertexList Predecessors;
	DAGVertexList Successors;


public:
	DAGNode() : varWidth(0), varDepth(0), opWidth(0), opDepth(0), visited(false), valueNode(nullptr), type(VAL) {}
	DAGNode(llvm::Value* value, vertex_t type) : varWidth(0), varDepth(0), 
		opWidth(0), opDepth(0), visited(false), valueNode(nullptr), llvmValue(value), type(type) {}
	~DAGNode() {}

	int getVarWidth() { return varWidth; }
	int getVarDepth() { return varDepth; }
	int getOpWidth() { return opWidth; }
	int getOpDepth() { return opDepth; }
	vertex_t getType() { return type; }
	const llvm::Value* getllvmValue() { return llvmValue; }
	llvm::Type* getllvmValueTy() { return llvmValue->getType(); }
	std::string getConstName() { return constName; }
	DAGNode* getValueNode() { return valueNode; }
	DAGVertexList& getSuccessors() { return Successors; }
	bool hasSuccessors() { return !Successors.empty(); }
	bool hasPredecessors() { return !Predecessors.empty(); }

	void setVarWidth(int width) { varWidth = width; }
	void setVarDepth(int depth) { varDepth = depth; }
	void setOpWidth(int width) { opWidth = width; }
	void setOpDepth(int depth) { opDepth = depth; }
	void setValueNode(DAGNode* node) { valueNode = node; }
	void setConstName(std::string name) { constName = name; }


	DAGNode* getSuccessor(DAGNode* successor);
	bool addSuccessor(DAGNode* successor);
	bool removeSuccessor(DAGNode* successor);
	bool addPredecessor(DAGNode* predecessor);

	std::string getName();
	bool setName(std::string name);
	void setVisited(bool status);
	void setAllUnvisited();

	bool hasStoredValue();
	bool hasBeenVisited();
	DAGNode* getStoredValueNode();

	bool isStoreInstNext();
	bool isSuccessorPresent(DAGNode* successor);
	bool isPredecessorPresent(DAGNode* predecessor);
	bool isaInst();

	void print();
	void prettyPrint(int tabCountLeft=0, int tabCountRight=2, int biOpCount=0);
	std::string generateTabs(int count);
	void printSuccessorsNames();
	void printPredecessorsNames();

	bool DOTcreateNode(std::ostringstream &outSS, DAGNode* parentNode=nullptr);
	std::string DOTnodeID();
};


#endif /* DAG_NODE_H */ 