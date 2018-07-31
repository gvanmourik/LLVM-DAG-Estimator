#ifndef DAG_BUILDER_H
#define DAG_BUILDER_H

#include "VDG.h"
#include "DAGNode.h"
#include "DepNode.h"


class DAGBuilder 
{
private:
	VariableDependencyGraph VDG;
	DAGVertexList DAGVertices;
	DAGNameList keyByName;
	int ID;
	int NameCount;
	bool hasBeenInitialized;
	bool DAGIsLocked;


public:
	DAGBuilder() : ID(0), NameCount(-1), hasBeenInitialized(false) {}
	~DAGBuilder() {}

	void lock() { DAGIsLocked = true; }

	//collectAdjNodes() and createDependenceGraph() must be called first
	int getVarWidth() 
	{ 
		VDG.lock();
		int width = VDG.getWidth();
		VDG.unlock();
		return width; 
	}
	int getVarDepth() 
	{ 
		VDG.lock();
		int depth = VDG.getDepth();
		VDG.unlock();
		return depth;
	}

	void init()
	{
		VDG.clear();
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

	/// DEPENDENCY GRAPH
	/**
	 * @brief      Creates a variable dependency graph from the DAG.
	 */
	void createDependenceGraph()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		for (int i = 0; i < ID; ++i)
		{
			traverseDAGNode(DAGVertices[i]);
		}
	}

private:
	void traverseDAGNode(DAGNode *node)
	{
		if ( node != nullptr )
		{	
			auto opCode = node->getOpcode();
			if ( opCode == Instruction::Store /*|| opCode == Instruction::Ret*/ )
			{
				// outs() << "building dependence unit... (" << node->getName() <<  ")\n";
				buildDependenceUnit(node);
			}
			traverseDAGNode(node->getLeft());
			traverseDAGNode(node->getRight());
		} 
	}

	void buildDependenceUnit(DAGNode *node)
	{
		auto opCode = node->getOpcode();
		if ( opCode == Instruction::Store )
		{
			// Add primaryOperand
			DAGNode *nodeSelector = node->getRight();
			const bool isOperator = true;
			DepNode* primaryOperand = addDep(nodeSelector, nullptr, !isOperator);

			// Add the list of ops on which the primary operator depends
			addDepWrapper(node->getLeft(), primaryOperand);
		}
		/// Are functions void??
		// if ( opCode == Instruction::Ret )
		// {
		// 	while ( !node->isValueOnlyNode() )
		// 		node = node->getLeft();
		// }
	}

	void addDepWrapper(DAGNode *node, DepNode* primaryOperand)
	{
		const bool isOperator = true;
		if ( node->isValueOnlyNode() )
		{
			/// Base case
			addDep(node, primaryOperand, !isOperator);
		}
		else if ( node->getOpcode() == Instruction::Load )
		{
			/// Collect Value of the load (node->right == nullptr)
			addDepWrapper(node->getLeft(), primaryOperand);
		}
		else
		{
			/// Binary operator case
			// add operator
			DepNode* Operator = addDep(node, primaryOperand, isOperator);

			addDepWrapper(node->getLeft(), Operator);
			addDepWrapper(node->getRight(), Operator);
		}
	}

	DepNode* addDep(DAGNode *node, DepNode* parentNode, bool isOperator)
	{
		DepNode *op;
		std::string name, opcodeName;
		// outs() << "Name = " << node->getName();
		// outs() << " (ID: " << node->getID() << ")\n";
		opcodeName = node->getOpcodeName();

		/// If node is an operator collect the name, otherwise find the value node
		if (isOperator)
			name = node->getName();
		else
			name = findValueNode(node);

		/// Check whether that op is already present as a dependency
		if ( VDG.isNamePresentDep(name) )
			op = VDG.getDepNodeByName(name);
		else
			op = newDepNode(name, opcodeName, isOperator);

		if (parentNode == nullptr)
			return op; // adding a parentNode

		parentNode->addOp(op, op->getID());
		return op; // adding a member
	}

	std::string findValueNode(DAGNode *operatorNode)
	{
		DAGNode *node = operatorNode;
		while ( !node->isValueOnlyNode() )
		{
			node = node->getLeft();				
		}
		return node->getName();
	}

	DepNode* newDepNode(std::string name, std::string opcodeName, bool isOperator)
	{
		DepNode* node = new DepNode(name, VDG.getDepID(), opcodeName, isOperator);
		VDG.setDepNode(node, name);
		return node;
	}

public:
	void fini()
	{
		// outs() << "Finalizing DAG build...\n";
		collectAdjNodes();
		// outs() << "Configuring dependence graph...\n";
		createDependenceGraph();
		// VDG.lock();
		// outs() << "Done...use print() and printDependencyGraph() to view.\n";

		// print();
		// printDependencyGraph();
		// outs() << "Width = " << variableWidth() << "\n";
		// outs() << "Depth = " << variableDepth() << "\n";
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
		VDG.lock();
		VDG.print();
		VDG.unlock();
	}
	

};


#endif /* DAG_BUILDER_H */