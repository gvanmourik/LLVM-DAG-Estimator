#ifndef DAG_NODE_H
#define DAG_NODE_H

#include <map>
#include <llvm/IR/Instruction.h>

class DAGNode;
typedef std::map<int, DAGNode*> DAGNodeList;

class DAGNode {

private:
	llvm::Instruction *Inst;
	llvm::Value *Val;
	unsigned opcode;
	int ID;
	DAGNode *left;
	DAGNode *right;
	DAGNodeList adjNodes;

public:
	DAGNode() : opcode(0), ID(-1) {}
	DAGNode(llvm::Instruction *inst, int id) : Inst(inst)
	{
		opcode = inst->getOpcode();
		ID = id;
		Val = nullptr;
		left = nullptr;
		right = nullptr;
	}
	DAGNode(llvm::Instruction *inst, llvm::Value *value, int id) : Inst(inst)
	{
		opcode = 0; //invalid opcode indicating value-only node
		ID = id;
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
	DAGNode* getAdjNode(int ID) { return adjNodes[ID]; }
	DAGNodeList getAdjNodes() { return adjNodes; }
	llvm::Instruction* getInst() { return Inst; }

	void setLeft(DAGNode *assignedNode) { left = assignedNode; }
	void setRight(DAGNode *assignedNode) { right = assignedNode; }
	void addAdjNode(DAGNode *node) { adjNodes[node->getID()] = node; }

	bool leftIsEmpty()
	{
		if (left == nullptr)
			return true;
		else 
			return false;
	}

	bool isEmpty()
	{
		if (left == nullptr && right == nullptr)
			return true;
		else
			return false;
	}

	void print()
	{
		if ( opcode != 0 )
			outs() << "\tInstruction: " << getOpcodeName() << " (opcode=" << opcode << ")";
		if ( Val != nullptr )
		{
			outs() << "\t      Value: ";

			if ( Val->hasName() )
				outs() << Val->getName();
			else
				outs() << "<unnamed value>";
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

#endif /* DAG_NODE_H */ 