#ifndef DAG_NODE_H
#define DAG_NODE_H

#include <map>
#include <llvm/IR/Instruction.h>

class DAGNode;
typedef std::map<int, DAGNode*> DAGNodeList;

class DAGNode {

private:
	llvm::Instruction *Inst;
	std::string Name;
	llvm::Value *Val;
	unsigned opcode;
	int ID;
	bool valueOnlyNode;
	DAGNode *left;
	DAGNode *right;
	DAGNodeList adjNodes;

public:
	DAGNode() : Inst(nullptr), opcode(0), ID(-1), valueOnlyNode(false) {}
	DAGNode(llvm::Value *value, int id)
	{
    if (llvm::isa<llvm::Instruction>(value) && !llvm::isa<llvm::AllocaInst>(value) )
		{
      Inst = llvm::cast<llvm::Instruction>(value);
			opcode = Inst->getOpcode();
			valueOnlyNode = false;
			Val = nullptr;
		}
		else
		{
			Inst = nullptr;
			opcode = 0; //invalid opcode indicating value-only node
			valueOnlyNode = true;
			Val = value;
		}
		ID = id;
		left = nullptr;
		right = nullptr;
	}
	~DAGNode() {}

	const char* getOpcodeName() 
	{ 
		if (Inst != nullptr)
			return Inst->getOpcodeName();
		else
			return "<no inst>"; 
	}

	int getID() { return ID; }
  llvm::Value* getValue() { return Val; }
	DAGNode* getLeft() { return left; }
	DAGNode* getRight() { return right; }
	DAGNode* getAdjNode(int ID) { return adjNodes[ID]; }
	DAGNodeList getAdjNodes() { return adjNodes; }
	llvm::Instruction* getInst() { return Inst; }
	std::string getName() { return Name; }
	unsigned getOpcode() { return opcode; }
	bool isValueOnlyNode() { return valueOnlyNode; }
	bool hasAdjVertices() { return !adjNodes.empty(); }

	void setAsInstNode() 
	{ 
		opcode = Inst->getOpcode();
		valueOnlyNode = false;
		Val = nullptr;
	}
	void setLeft(DAGNode *assignedNode) { left = assignedNode; }
	void setRight(DAGNode *assignedNode) { right = assignedNode; }
	void setValue(llvm::Value *val) { Val = val; }
	void addAdjNodes(DAGNode *node) 
	{ 
		if ( node != nullptr )
		{
			adjNodes[node->getID()] = node;
			addAdjNodes( node->getLeft() );
			addAdjNodes( node->getRight() );
		}
	}
	void setName(std::string name) { Name = name; }

	bool leftIsEmpty()
	{
		if (left == nullptr)
			return true;
		else 
			return false;
	}

	void print()
	{
		if ( !valueOnlyNode )
		{
      llvm::outs() << "\tInstruction: " << getOpcodeName() << " (opcode=" << opcode << ")";
      llvm::outs() << " Name = " << Name;
		}
		else
		{
      llvm::outs() << "\t      Value: ";
			if ( Val->hasName() )
        llvm::outs() << Val->getName();
			else
        llvm::outs() << Name;
		}
    llvm::outs() << " (ID: " << ID << ")\n";
		if ( left != nullptr )
			left->print();
		if ( right != nullptr )
			right->print();
	}
	
	void printAdjNodeIDs()
	{
		for (auto iter=adjNodes.begin(); iter!=adjNodes.end(); ++iter)
		{
      llvm::outs() << "Node" << iter->second->ID << " ";
		}
    llvm::outs() << "\n";
	}

};

typedef std::unordered_map<int, DAGNode*> DAGVertexList;
typedef std::unordered_map<std::string, int> DAGNameList;

#endif /* DAG_NODE_H */ 
