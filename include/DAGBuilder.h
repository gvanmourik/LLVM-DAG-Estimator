#ifndef DAG_BUILDER_H
#define DAG_BUILDER_H

#include "DGBuilder.h"
#include "DAGNode.h"
#include "DepNode.h"

typedef std::unordered_map<std::string, llvm::Instruction*> DAGInstTracker;

class DAGBuilder 
{
private:
	DGBuilder *dgBuilder; //dependence graph builder
	DAGVertexList DAGVertices;
	DAGNameList keyByName;
	DAGInstTracker instByName;
	int ID;
	int NameCount;
	bool hasBeenInitialized;
	bool DAGIsLocked;


public:
	DAGBuilder() : dgBuilder(nullptr), ID(0), NameCount(0), hasBeenInitialized(false) {}
	~DAGBuilder() {}

	void lock() { DAGIsLocked = true; }
	DGBuilder* getDGBuilder() 
	{ 
		assert( dgBuilder != nullptr && "DGBuilder has not been set. Use createDG()");
		return dgBuilder;
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

	void addOperator(llvm::Instruction *inst)
	{
		DAGNode *operatorNode;
		
		// if inst has name, then it will be unique
		if ( inst->hasName() )
		{
			operatorNode = addVertex(inst, inst->getName());
			// outs() << "instName = " << inst->getName() << "\n";
			addOperands(inst, operatorNode);
		}
		else
		{
			std::string operatorName = genNameForValue(inst);

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
		// llvm::Value could be an instruction or const value
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
			valueName = genNameForValue(value);


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

	std::string genNameForValue(llvm::Value *value)
	{
		std::string operatorName;
		if ( isa<Instruction>(value) )
		{
			auto inst = cast<Instruction>(value);
			if ( isa<StoreInst>(inst) )
			{
				operatorName = inst->getOperand(1)->getName();
				operatorName = "store_" + operatorName;
				// outs() << "storeName = " << operatorName << "\n";
			}
			if ( isa<LoadInst>(inst) )
			{
				operatorName = inst->getOperand(0)->getName();
				operatorName = "load_" + operatorName;
				// outs() << " loadName = " << operatorName << "\n";
			}
		}
		else
		{
			operatorName = genName();
			// outs() << " genName = " << operatorName << "\n";
		}
		return operatorName;
	}

	std::string genName()
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
		dgBuilder = new DGBuilder(DAGVertices, ID);
		
		dgBuilder->createDG();
		dgBuilder->createVDG();
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
		dgBuilder->printDG();
	}
	

};


#endif /* DAG_BUILDER_H */