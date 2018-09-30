#ifndef DAG_NODE_H
#define DAG_NODE_H

#include <unordered_map>
#include <llvm/IR/Instruction.h>

class DAGNode;
typedef enum vertex_t{VAL,INST} vertex_t;
typedef std::unordered_map<DAGNode*, DAGNode*> DAGVertexList;
typedef std::unordered_map<llvm::Value*, DAGNode*> DAGValueList;
// typedef std::unordered_map<std::string, DAGNode*> DAGNameList;
// typedef std::map<int, DAGNode*> DAGNodeList;

class DAGNode {

private:
	// llvm::Instruction *Inst;
	// std::string Name;
	// llvm::Value *Val;
	// unsigned opcode;
	// int ID;
	// bool valueOnlyNode;
	// DAGNode *left;
	// DAGNode *right;
	// DAGNodeList adjNodes;
	
	int varWidth;
	int varDepth;
	int opWidth;
	int opDepth;
	DAGNode* valueNode;
	llvm::Value* content; //holds either instruction or value
	vertex_t type;
	std::string constName;
	DAGVertexList Successors;


public:
	DAGNode() : varWidth(1), varDepth(1), opWidth(1), opDepth(1), valueNode(nullptr), type(VAL) {}
	DAGNode(llvm::Value* value, vertex_t type) : varWidth(1), varDepth(1), 
		opWidth(1), opDepth(1), valueNode(nullptr), content(value), type(type) {}
	~DAGNode() {}

	int getVarWidth() { return varWidth; }
	int getVarDepth() { return varDepth; }
	int getOpWidth() { return opWidth; }
	int getOpDepth() { return opDepth; }
	vertex_t getType() { return type; }
	llvm::Type* getContentTy() { return content->getType(); }
	std::string getConstName() { return constName; }
	DAGVertexList getSuccessors() { return Successors; }
	DAGNode* getValueNode() { return valueNode; }
	bool hasSuccessors() { return !Successors.empty(); }

	void setVarWidth(int width) { varWidth = width; }
	void setVarDepth(int depth) { varDepth = depth; }
	void setOpWidth(int width) { opWidth = width; }
	void setOpDepth(int depth) { opDepth = depth; }
	void setValueNode(DAGNode* node) { valueNode = node; }
	void setConstName(std::string name) { constName = name; }

	/// Best to first check if successor is present to avoid returning null
	DAGNode* getSuccessor(DAGNode* successor)
	{
		if ( isSuccessorPresent(successor) )
			return Successors[successor];
		else
			return nullptr;
	}

	std::string getName() 
	{
		if ( !constName.empty() )
			return constName;
		else
		{
			if ( content->hasName() )
				return content->getName();
			else
				return "<no name>"; 
		}
	}

	bool setName(std::string name)
	{
		if ( content->hasName() )
		{
			return false;
		}
		content->setName(name);
		return true;
	}

	bool addSuccessor(DAGNode* successor)
	{
		if ( !isSuccessorPresent(successor) ) 
		{
			Successors[successor] = successor;
			return true;
		}
		return false;
	}

	bool removeSuccessor(DAGNode* successor)
	{
		if ( isSuccessorPresent(successor) ) 
		{
			Successors.erase(successor);
			return true;
		}
		return false;
	}

	// void removeSuccessors() { Successors.clear(); }

	bool isSuccessorPresent(DAGNode* successor)
	{
		if (Successors.count(successor) != 0)
			return true;
		else
			return false;
	}

	bool isaInst()
	{
		if (type == INST)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// int getID() { return ID; }
 	// llvm::Value* getValue() { return Val; }
	// DAGNode* getLeft() { return left; }
	// DAGNode* getRight() { return right; }
	// DAGNode* getAdjNode(int ID) { return adjNodes[ID]; }
	// DAGNodeList getAdjNodes() { return adjNodes; }
	// llvm::Instruction* getInst() { return Inst; }
	// std::string getName() { return Name; }
	// unsigned getOpcode() { return opcode; }
	// bool isValueOnlyNode() { return valueOnlyNode; }
	// bool hasAdjVertices() { return !adjNodes.empty(); }

	// void setAsInstNode() 
	// { 
	// 	opcode = Inst->getOpcode();
	// 	valueOnlyNode = false;
	// 	Val = nullptr;
	// }
	// void setLeft(DAGNode *assignedNode) { left = assignedNode; }
	// void setRight(DAGNode *assignedNode) { right = assignedNode; }
	// void setValue(llvm::Value *val) { Val = val; }
	// void addAdjNodes(DAGNode *node) 
	// { 
	// 	if ( node != nullptr )
	// 	{
	// 		adjNodes[node->getID()] = node;
	// 		addAdjNodes( node->getLeft() );
	// 		addAdjNodes( node->getRight() );
	// 	}
	// }
	// void setName(std::string name) { Name = name; }

	// bool leftIsEmpty()
	// {
	// 	if (left == nullptr)
	// 		return true;
	// 	else 
	// 		return false;
	// }

	void print()
	{
		if (type == INST)
		{
			auto inst = llvm::cast<llvm::Instruction>(content);
      		llvm::outs() << "\tInstruction: " << inst->getOpcodeName() << " (opcode=" << inst->getOpcode() << ")";
      		llvm::outs() << " Name = " << getName();
		}
		else
		{
      		llvm::outs() << "\t      Value: " << getName();
      	}	
    	llvm::outs() << " (ID: " << &*content << ")\n";
		
		/// Recursively print each successor
		DAGNode* successor;
		for (auto successor_pair : Successors)
		{
			successor = successor_pair.second;
			successor->print();
		}
	}
	
	void printSuccessorsNames()
	{
		for (auto successor_pair : Successors)
		{
      		llvm::outs() << successor_pair.second->getName() << " ";
		}
    	llvm::outs() << "\n";
	}

};

#endif /* DAG_NODE_H */ 
