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
	DAGNode(llvm::Instruction *inst, int id) : Inst(inst)
	{
		opcode = inst->getOpcode();
		ID = id;
		valueOnlyNode = false;
		Val = nullptr;
		left = nullptr;
		right = nullptr;
	}
	DAGNode(llvm::Instruction *inst, llvm::Value *value, int id) : Inst(inst)
	{
		opcode = 0; //invalid opcode indicating value-only node
		ID = id;
		valueOnlyNode = true;
		Val = value;
		left = nullptr;
		right = nullptr;
	}
	~DAGNode() 
	{
		delete left;
		delete right;
	}

	const char* getOpcodeName() 
	{ 
		if (Inst != nullptr)
			return Inst->getOpcodeName();
		else
			return "<no inst>"; 
	}

	int getID() { return ID; }
	Value* getValue() { return Val; }
	DAGNode* getLeft() { return left; }
	DAGNode* getRight() { return right; }
	DAGNode* getAdjNode(int ID) { return adjNodes[ID]; }
	DAGNodeList getAdjNodes() { return adjNodes; }
	llvm::Instruction* getInst() { return Inst; }
	std::string getName() { return Name; }
	unsigned getOpcode() { return opcode; }
	bool isValueOnlyNode() { return valueOnlyNode; }

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

	bool rightIsEmpty()
	{
		if (right == nullptr)
			return true;
		else 
			return false;
	}

	void print()
	{
		if ( !valueOnlyNode )
		{
			outs() << "\tInstruction: " << getOpcodeName() << " (opcode=" << opcode << ")";
			outs() << " Name = " << Name;
		}
		else
		{
			outs() << "\t      Value: ";
			if ( Val->hasName() )
				outs() << Val->getName();
			else
				outs() << Name;
		}
		outs() << " (ID: " << ID << ")\n";
		if ( left != nullptr )
			left->print();
		if ( right != nullptr )
			right->print();
	}

	void printInst()
	{
		for (auto iter=Inst->op_begin(); iter!=Inst->op_end(); ++iter)
		{
			outs() << "\t Count" << iter->getOperandNo() << ": " << iter->get()->getValueID();
			outs() << " (" << Inst->getOpcodeName( iter->get()->getValueID() ) << ")\n";
		}
	}

	void printAdjNodes()
	{
		for (auto iter=adjNodes.begin(); iter!=adjNodes.end(); ++iter)
		{
			iter->second->print();
		}
	}

	void printAdjNodeIDs()
	{
		for (auto iter=adjNodes.begin(); iter!=adjNodes.end(); ++iter)
		{
			outs() << "Node" << iter->second->ID << " ";
		}
		outs() << "\n";
	}

};

typedef std::unordered_map<int, DAGNode*> DAGVertexList;
typedef std::unordered_map<std::string, int> DAGNameList;

#endif /* DAG_NODE_H */ 