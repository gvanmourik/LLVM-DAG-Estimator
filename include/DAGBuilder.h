#ifndef DAG_BUILDER_H
#define DAG_BUILDER_H

#include "VDGBuilder.h"
#include "DAGNode.h"
#include "DepNode.h"


class DAGBuilder 
{
private:
	VDGBuilder *vdgBuilder;
	DAGVertexList DAGVertices;
	DAGNameList keyByName;
	int ID;
	int NameCount;
	bool hasBeenInitialized;
	bool DAGIsLocked;


public:
	DAGBuilder() : vdgBuilder(nullptr), ID(0), NameCount(-1), hasBeenInitialized(false) {}
	~DAGBuilder() {}

	void lock() { DAGIsLocked = true; }
	VariableDependencyGraph getVDG() 
	{ 
		assert( vdgBuilder != nullptr && "VDGBuilder has not been set. Use createDependenceGraph()");
		return vdgBuilder->getVDG();
	}

	void init()
	{
		DAGVertices.clear();
		keyByName.clear();
		DAGIsLocked = false;
		hasBeenInitialized = true;
	}

	/// DAG
	bool add(llvm::Instruction *inst)
	{
		assert( hasBeenInitialized && "Builder has not been initialized! Do so with init()");
		/// Check if instruction is invalid operator (aka branch)
		if ( isInvalidOperator(inst) )
			return false;
		if ( isBranch(inst) )
			return false;
		if ( isAlloca(inst) )
			return false;

		/// Add operator
		DAGNode *instNode = addVertex(inst);

		/// Add operands
		auto iter = inst->op_begin();
		llvm::Value *val = iter->get();
		if ( hasTwoOperands(inst) )
		{
			addOperand(val, inst, instNode); //first
			
			iter++;
			val = iter->get();
			addOperand(val, inst, instNode); //second
		}
		else
		{
			addOperand(val, inst, instNode);
		}

		return true;
	}

private:
	void addEdge(DAGNode *parentNode, DAGNode *childNode)
	{
		if ( childNode != nullptr ) 
		{
			if ( parentNode->leftIsEmpty() )
				parentNode->setLeft(childNode);
			else
				parentNode->setRight(childNode);
		}
	}
	
	DAGNode* addVertex(llvm::Instruction *inst)
	{
		DAGVertices[ID] = new DAGNode(inst, ID);
		if ( inst->hasName() )
			DAGVertices[ID]->setName( inst->getName() );
		else
			DAGVertices[ID]->setName( genName() );
		
		keyByName[ DAGVertices[ID]->getName() ] = ID;
		DAGNode *node = DAGVertices[ID];
		ID++;

		return node;
	}

	void addVertex(llvm::Value *value, llvm::Instruction *inst, std::string name)
	{
		DAGVertices[ID] = new DAGNode(inst, value, ID);
		DAGVertices[ID]->setName(name);

		keyByName[name] = ID;
		ID++;
	}

	void addOperand(llvm::Value *value, llvm::Instruction *inst, DAGNode* instNode)
	{
		DAGNode *operandNode;
		std::string name = getNewName(value);

		if ( !isNamePresent(name) )
			addVertex(value, inst, name);
		else
			resetNameCount(value); //revert count

		operandNode = DAGVertices[keyByName[name]];
		addEdge(instNode, operandNode);
	}

	std::string getNewName(llvm::Value *target)
	{
		if ( target->hasName() )
			return target->getName();
		else 
			return genName();
	}

	std::string genName()
	{
		NameCount++;
		return "Val" + std::to_string(NameCount);
	}

	void resetNameCount(llvm::Value *target)
	{
		if ( !target->hasName() )
			NameCount--;
	}
	
	bool isNamePresent(std::string name)
	{
		if( keyByName.count(name) == 0 )
			return false;
		else
			return true;
	}
			
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
		if (inst->getOpcode() == Instruction::Br)
			return true;
		else
			return false;
	}

	bool isAlloca(llvm::Instruction *inst)
	{
		if (inst->getOpcode() == Instruction::Alloca)
			return true;
		else
			return false;
	}

public:
	void collectAdjNodes()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		for (int i = 0; i < ID; ++i)
		{
			DAGVertices[i]->addAdjNodes( DAGVertices[i]->getLeft() );
			DAGVertices[i]->addAdjNodes( DAGVertices[i]->getRight() );
		}
	}

	void createVDG()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		vdgBuilder = new VDGBuilder(DAGVertices, ID);
		vdgBuilder->createVDG();
	}

	void fini()
	{
		collectAdjNodes();
		createVDG();
	}

	void print()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		outs() << "\nDAG Nodes:\n";
		for (int i = 0; i < ID; ++i)
		{
			outs() << "-------------------------------------------------------------\n";
			outs() << "Node" << i << ":\n";
			DAGVertices[i]->print();
			outs() << "       Adjacent Nodes: ";
			DAGVertices[i]->printAdjNodeIDs();
			outs() << "-------------------------------------------------------------\n";
		}
		outs() << "\n";
	}

	void printDependencyGraph()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		vdgBuilder->printVDG();
	}
	

};


#endif /* DAG_BUILDER_H */