#ifndef DAG_BUILDER_H
#define DAG_BUILDER_H

#include <queue>
#include "DAGNode.h"
#include "DepNode.h"


class DAGBuilder {

private:
	DAGVertexList DAGVertices;
	DepVertexList DependenceVertices;
	DAGNameList keyByName;
	DepNameList depKeyByName;
	int ID;
	int NameCount;
	int DepID; // for dependence graph
	bool hasBeenInitialized;
	bool DAGIsLocked;


public:
	DAGBuilder() : ID(0), NameCount(-1), DepID(0), hasBeenInitialized(false) {}
	~DAGBuilder() {}

	void lock() { DAGIsLocked = true; }

	//collectAdjNodes() and createDependenceVertices() must be called first
	int getVarDepWidth() { return variableWidth(); }
	int getVarDepDepth() { return variableDepth(); }

	void init()
	{
		DAGVertices.clear();
		DependenceVertices.clear();
		keyByName.clear();
		depKeyByName.clear();
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
		/// 2 is the Opcode for br
		if (inst->getOpcode() == 2)
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
			/// Store has opcode = 31
			if ( node->getOpcode() == 31 )
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
		if ( node->getLeft()->isValueOnlyNode() )
		{
			// Add primaryOperand
			DepNode* primaryOperand = addDep(node->getRight(), nullptr, false);

			// Add value on which the primaryOperand depends
			addDep(node->getLeft(), primaryOperand, false);
		}
		else
		{
			// Add primaryOperand
			DAGNode *nodeSelector = node->getRight();
			bool isOperator = false;
			DepNode* primaryOperand = addDep(nodeSelector, nullptr, isOperator);

			// Add operator
			nodeSelector = node->getLeft();
			DepNode* depOperator = addDep(nodeSelector, primaryOperand, true);

			// Add operands on which the primaryOperand depends
			addDep(nodeSelector->getLeft(), depOperator, isOperator);
			addDep(nodeSelector->getRight(), depOperator, isOperator);
		}
	}

	DepNode* addDep(DAGNode *node, DepNode* parentNode,  bool isOperator)
	{
		DepNode *op;
		std::string name, opcodeName;
		opcodeName = node->getOpcodeName();

		if (isOperator)
			name = node->getName();
		else
			name = findValueName(node);

		if ( isNamePresentDep(name) )
			op = DependenceVertices[depKeyByName[name]];
		else
			op = newDepNode(name, opcodeName, isOperator);

		if (parentNode == nullptr)
			return op; // adding a parentNode

		parentNode->addOp(op, op->getID());
		return op; // adding a member
	}

	std::string findValueName(DAGNode *operatorNode)
	{
		DAGNode *node = operatorNode;
		while ( !node->isValueOnlyNode() )
		{
			node = node->getLeft();				
		}
		return node->getName();
	}

	bool isNamePresentDep(std::string name)
	{
		if( depKeyByName.count(name) == 0 )
			return false;
		else
			return true;
	}

	DepNode* newDepNode(std::string name, std::string opcodeName, bool isOperator)
	{
		DepNode* node = new DepNode(name, DepID, opcodeName, isOperator);
		DependenceVertices[DepID] = node;
		depKeyByName[name] = DepID;
		DepID++;
		return node;
	}

	/// TRAVERSAL
	int variableWidth()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		DepNode *root = findDepRoot();
		std::queue<DepNode*> wavefront;
		wavefront.push(root);

		int maxWidth = 1; // to avoid 1st assignment in while
		int width = 0;
		DepNode *selector;
		DepNodeList ops;
		while ( !wavefront.empty() )
		{
			width = wavefront.size();
			if ( maxWidth < width )
				maxWidth = width;

			selector = wavefront.front();
			wavefront.pop();

			if ( selector->hasDependents() )
			{
				ops = selector->getOps();
				for (auto iter=ops.begin(); iter!=ops.end(); ++iter)
				{
					wavefront.push(iter->second);
				}
			}
		}
		return maxWidth;
	}

	int variableDepth()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		DepNode *root = findDepRoot();
		int maxDepth = 0;
		int depthCount = 1;
		depthWrapper(root, depthCount, maxDepth);

		return maxDepth;
	}

	void depthWrapper(DepNode *node, int count, int &maxCount)
	{
		auto Ops = node->getOps();
		if ( !node->isAnOperator() )
			count++;

		for (auto iter=Ops.begin(); iter!=Ops.end(); ++iter)
		{
			if ( maxCount < count )
			{
				maxCount = count;
			}
			depthWrapper(iter->second, count, maxCount);
		}
	}

	DepNode* findDepRoot()
	{
		DepNode *currentNode;
		for (int i = 0; i < DepID; ++i)
		{
			currentNode = DependenceVertices[i];
			if ( currentNode->hasDependents() && !currentNode->isADependent() )
				return currentNode;
		}
		return nullptr;
	}

public:
	void fini()
	{
		// outs() << "Finalizing DAG build...\n";
		collectAdjNodes();
		// outs() << "Configuring dependence graph...\n";
		createDependenceGraph();
		// outs() << "Done...use print() and printDependencyGraph() to view.\n";

		// print();
		// printDependencyGraph();
		// outs() << "Width = " << variableWidth() << "\n";
		// outs() << "Depth = " << variableDepth() << "\n";
	}

	void print()
	{
		outs() << "\nDAG Nodes:\n";
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

	void printDependencyGraph()
	{
		outs() << "\nDependency Nodes:\n";
		for (int i = 0; i < DepID; ++i)
		{
			outs() << "---------------------------------------------------\n";
			outs() << "DepNode" << i << ":\n";
			DependenceVertices[i]->print();
			outs() << "---------------------------------------------------\n";
		}
		outs() << "\n";
	}
	

};


#endif /* DAG_BUILDER_H */