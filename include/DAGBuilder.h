#ifndef DAG_BUILDER_H
#define DAG_BUILDER_H

#include "DAGNode.h"

typedef std::unordered_map<int, DAGNode*> DAGVertexList;
typedef std::unordered_map<Value*, int> DAGValueList;


class DAGBuilder {

private:
	DAGVertexList DAGVertices;
	DAGValueList keyByValue;
	int ID;


public:
	DAGBuilder() : ID(0) {}
	~DAGBuilder() {}

	DAGNode* getRoot() { return DAGVertices[0]; }

	bool add(llvm::Instruction *inst)
	{
		/// Check if instruction is invalid operator (aka branch)
		if ( isInvalidOperator(inst) )
			return false;
		if ( isBranch(inst) )
			return false;

		/// Add operator
		addVertex(inst);

		/// Add operands
		if ( hasTwoOperands(inst) )
		{
			llvm::Value *valA = inst->op_begin()->get();
			llvm::Value *valB = inst->op_end()->get();

			if ( !isValPresent(valA) )
				addVertex(inst, valA);
			// Access the parent node with ID-2 because of the two
			// newly added vertices.
			addEdge(DAGVertices[ID-2], DAGVertices[keyByValue[valA]]);

			if ( !isValPresent(valB) )
				addVertex(inst, valB);
			addEdge(DAGVertices[ID-2], DAGVertices[keyByValue[valB]]);
		}
		else
		{
			llvm::Value *valA = inst->op_begin()->get();

			if ( !isValPresent(valA) )
				addVertex(inst, valA);
			// Access the parent node with ID-2 because of the two
			// newly added vertices.
			addEdge(DAGVertices[ID-2], DAGVertices[keyByValue[valA]]);
		}

		return true;
	}

	void addEdge(DAGNode *parentNode, DAGNode *childNode)
	{
		parentNode->addAdjNode(childNode);
		childNode->addAdjNode(parentNode);

		if ( parentNode->isEmpty() )
		{
			if ( parentNode->leftIsEmpty() )
				parentNode->setLeft(childNode);
			else
				parentNode->setRight(childNode);
		}
	}
	
	void addVertex(llvm::Instruction *inst)
	{
		DAGVertices[ID] = new DAGNode(inst, ID);
		ID++;
	}

	void addVertex(llvm::Instruction *inst, llvm::Value *value)
	{
		DAGVertices[ID] = new DAGNode(inst, value, ID);
		keyByValue[value] = ID;
		ID++;
	}
	
	bool isValPresent(llvm::Value* value)
	{
		if( keyByValue.count(value) == 0 )
			return false;
		else
			return true;
	}
			
	/// Check if the instruction has two operands (zeroth index is 0)
	bool hasTwoOperands(llvm::Instruction *inst)
	{
		int count = 0;
		for (auto iter=inst->op_begin(); iter!=inst->op_end(); ++iter)
			count++;

		if (count == 2)
			return true;
		else
			return false;
	}

	bool isInvalidOperator(llvm::Instruction *inst)
	{
		const char* opName = inst->getOpcodeName(inst->op_begin()->get()->getValueID());
		if (strcmp(opName, "<Invalid operator>") == 0)
			return true;
		else
			return false;
	}

	bool isBranch(llvm::Instruction *inst)
	{
		/// 2 is the Opcode for br
		if (inst->getOpcode() == 2)
			return true;
		else
			return false;
	}

	void print()
	{
		outs() << "\n";
		for (int i = 0; i < ID; ++i)
		{
			outs() << "---------------------------------------------------\n";
			outs() << "Node" << i << ":\n";
			DAGVertices[i]->print();
			outs() << "     Adjacent Nodes: ";
			DAGVertices[i]->printAdjNodeIDs();
			outs() << "---------------------------------------------------\n";
		}
		outs() << "\n";
	}
	

};


#endif /* DAG_BUILDER_H */