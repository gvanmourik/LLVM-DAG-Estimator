#ifndef DAG_BUILDER_H
#define DAG_BUILDER_H

#include "DGBuilder.h"

typedef std::unordered_map<std::string, llvm::Instruction*> DAGInstTracker;


class DAGBuilder 
{
private:
	DGBuilder *DG_Builder; //dependence graph builder
	DAGVertexList DAGVertices;
	DAGNameList keyByName;
	DAGInstTracker instByName;
	int ID;
	int NameCount;
	bool hasBeenInitialized;
	bool DAGIsLocked;


public:
	DAGBuilder() : DG_Builder(nullptr), ID(0), NameCount(0), hasBeenInitialized(false) {}
	~DAGBuilder() {}

	void lock() { DAGIsLocked = true; }
	DGBuilder* getDGBuilder() 
	{ 
		assert( DG_Builder != nullptr && "DGBuilder has not been set. Use createDG()");
		return DG_Builder;
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
		/// Ignore the following
		if ( isInvalidOperator(inst) )
			return false;
		if ( isBranch(inst) )
			return false;
		if ( isAlloca(inst) )
			return false;
		if ( isReturn(inst) )
			return false;

		addOperator(inst);

		return true;
	}

private:
	void addOperator(llvm::Instruction *inst)
	{
		DAGNode *operatorNode;
		// if instruction has name, then it WILL be unique
		if ( inst->hasName() )
		{
			operatorNode = addVertex(inst, inst->getName());
			addOperands(inst, operatorNode);
		}
		else
		{
			std::string operatorName = genValueName(inst);

			if ( !isNamePresent(operatorName) )
			{
				operatorNode = addVertex(inst, operatorName);
				addOperands(inst, operatorNode);
			}
		}
	}

	void addOperands(llvm::Instruction *inst, DAGNode *parentNode)
	{
		/// Add operands
		// llvm::Value could be either an instruction or const value
		llvm::Value *firstValue, *secondValue;
		firstValue = inst->getOperand(0);
		if ( hasTwoOperands(inst) )
		{
			secondValue = inst->getOperand(1);
			addOperand(firstValue, parentNode); //first
			addOperand(secondValue, parentNode); //second
		}
		else
		{
			addOperand(firstValue, parentNode);
		}
	}

	void addOperand(llvm::Value *value, DAGNode* parentNode)
	{
		DAGNode *valueNode;

		std::string valueName;
		if ( value->hasName() )
			valueName = value->getName();
		else
			valueName = genValueName(value);


		if ( !isNamePresent(valueName) )
			addVertex(value, valueName);

		valueNode = DAGVertices[keyByName[valueName]];
		addEdge(parentNode, valueNode);
	}

	DAGNode* addVertex(llvm::Value *value, std::string name)
	{
		DAGVertices[ID] = new DAGNode(value, ID);
		DAGVertices[ID]->setName(name);

		keyByName[name] = ID;
		ID++;

		return DAGVertices[keyByName[name]];
	}

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

	std::string genValueName(llvm::Value *value)
	{
		std::string operatorName;
		if ( isa<Instruction>(value) )
		{
			auto inst = cast<Instruction>(value);
			if ( isa<StoreInst>(inst) )
			{
				operatorName = inst->getOperand(1)->getName();
				operatorName = "store_" + operatorName;
			}
			if ( isa<LoadInst>(inst) )
			{
				operatorName = inst->getOperand(0)->getName();
				operatorName = "load_" + operatorName;
			}
		}
		else
		{
			operatorName = genConstName();
		}
		return operatorName;
	}

	std::string genConstName()
	{
		NameCount++;
		return "val_" + std::to_string(NameCount);
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

	bool isReturn(llvm::Instruction *inst)
	{
		if (inst->getOpcode() == Instruction::Ret)
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

	void createDG()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		DG_Builder = new DGBuilder(DAGVertices, ID);
		
		DG_Builder->createDG();
		DG_Builder->createVDG();
		// DG_Builder->createODG();
	}

	void fini()
	{
		collectAdjNodes();
		createDG();
		// createVDG();
	}

	void print()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		outs() << "\nDAG Nodes:\n";
		DAGNode *DAG_vertex;
		for (int i = 0; i < ID; ++i)
		{
			DAG_vertex = DAGVertices[i];
			outs() << "-------------------------------------------------------------\n";
			outs() << "Node" << i << ":\n";
			DAG_vertex->print();
			if ( DAG_vertex->hasAdjVertices() )
			{
				outs() << "     Adjacent Nodes: ";
				DAGVertices[i]->printAdjNodeIDs();
			}
			outs() << "-------------------------------------------------------------\n";
		}
		outs() << "\n";
	}

	void printDependencyGraph()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		DG_Builder->printDG();
	}
	

};


#endif /* DAG_BUILDER_H */