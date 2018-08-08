#ifndef VDG_BUILDER_H
#define VDG_BUILDER_H

#include <deque>
#include "DepNode.h"
#include "DAGNode.h"


class DependencyGraph 
{
friend class DGBuilder;
private:
	DepVertexList DependenceVertices;
	DepNameList keyByName;
	int ID;
	bool DGIsLocked;

public:
	DependencyGraph(): ID(0), DGIsLocked(false) {}
	~DependencyGraph() {}

	void unlock() { DGIsLocked = false; }
	void lock() { DGIsLocked = true; }
	
	bool isLocked() { return DGIsLocked; }
	int getNodeCount() { return ID; }
	int getWidth() { return findWidth(); }
	int getDepth() { return findDepth(); }
	std::string getDepNodeName(int ID) { return DependenceVertices[ID]->getName(); }
	DepNode* getDepNode(int ID) { return DependenceVertices[ID]; }
	DepNode* getDepNodeByName(std::string name) { return DependenceVertices[keyByName[name]]; }

protected:
	void setDepNode(DepNode* node, std::string name) 
	{
		DependenceVertices[ID] = node;
		keyByName[name] = ID;
		ID++;
	}

	bool isNamePresent(std::string name)
	{
		if( keyByName.count(name) == 0 )
			return false;
		else
			return true;
	}

	DepNode* findDepRoot()
	{
		DepNode *currentNode;
		for (int i = 0; i < ID; ++i)
		{
			currentNode = DependenceVertices[i];
			if ( currentNode->hasDependents() && !currentNode->isADependent() )
				return currentNode;
		}
		return nullptr;
	}

private:
	/// TRAVERSAL
	int findWidth()
	{
		assert( DGIsLocked && 
				"Dependence graph(DG) has not been locked! Do so with lock()");
		DepNode *root = findDepRoot();
		if ( root == nullptr )
			return 0;

		std::deque<DepNode*> wavefront;
		wavefront.push_back(root);

		int maxWidth = 1; // to avoid 1st assignment in while
		int width = 0;
		DepNode *selector;
		DepNodeList ops;
		while ( !wavefront.empty() )
		{
			width = wavefront.size();
			// width = wavefrontVarWidth(wavefront);
			if ( maxWidth < width )
				maxWidth = width;

			selector = wavefront.front();
			wavefront.pop_front();

			if ( selector->hasDependents() )
			{
				ops = selector->getOps();
				for (auto iter=ops.begin(); iter!=ops.end(); ++iter)
				{
					if ( iter->second->hasNotBeenVisited() )
					{
						wavefront.push_back(iter->second);
						iter->second->markAsVisited();
					}
				}
			}
		}
		return maxWidth;
	}

	// int wavefrontVarWidth(std::deque<DepNode*> &wavefront) const
	// {
	// 	int variableCount = 0;
	// 	for (auto node=wavefront.begin(); node!=wavefront.end(); ++node)
	// 	{
	// 		if ( !(*node)->isAnOperator() )
	// 		{
	// 			outs() << (*node)->getName() 
	// 			variableCount++;
	// 		}
	// 	}
	// 	return variableCount;
	// }

	int findDepth()
	{
		assert( DGIsLocked && 
				"Dependence graph(DG) has not been locked! Do so with lock()");
		DepNode *root = findDepRoot();
		if ( root == nullptr )
			return 0;

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

public:
	void clear() 
	{ 
		DependenceVertices.clear();
		keyByName.clear();  
	}

	bool empty() { return DependenceVertices.empty(); }

	void print()
	{
		assert( DGIsLocked && 
				"Dependence graph(DG) has not been locked! Do so with lock()");
    llvm::outs() << "\nDependency Nodes:\n";
		for (int i = 0; i < ID; ++i)
		{
      llvm::outs() << "---------------------------------------------------\n";
      llvm::outs() << "DepNode" << i << ":\n";
			DependenceVertices[i]->print();
      llvm::outs() << "---------------------------------------------------\n";
		}
    llvm::outs() << "\n";
	}
};


class DGBuilder
{
private:
	DependencyGraph DG;
	DependencyGraph VDG; //variable dependence graph
	// DependencyGraph ODG;
	DAGVertexList DAGVertices;
	const int DAGNodeCount;


public:
	DGBuilder(DAGVertexList &DAGVertices, int DAGNodeCount) :
		DAGVertices(DAGVertices), DAGNodeCount(DAGNodeCount) {}
	~DGBuilder() {}

	void clearDG() { DG.clear(); }
	void createDG()
	{
		assert( !DG.isLocked() && 
				"Dependence graph(DG) is locked! No changes can be made until it is unlocked.");
		for (int i = 0; i < DAGNodeCount; ++i)
		{
			traverseDAGNode(DAGVertices[i]);
		}
	}

	DependencyGraph getDG() 
	{
		DG.lock();
		auto DG_copy = DG;
		DG.unlock();
		return DG_copy;
	}

	DependencyGraph getVDG() 
	{
		VDG.lock();
		auto VDG_copy = VDG;
		VDG.unlock();
		return VDG_copy;
	}

	// DependencyGraph getDG() 
	// {
	// 	DG.lock();
	// 	auto DG_copy = DG;
	// 	DG.unlock();
	// 	return DG_copy;
	// }


private:
	void traverseDAGNode(DAGNode *node)
	{
		if ( node != nullptr )
		{	
			auto opCode = node->getOpcode();
      if ( opCode == llvm::Instruction::Store )
			{
				buildDependenceUnit(node);
			}
			traverseDAGNode(node->getLeft());
			traverseDAGNode(node->getRight());
		} 
	}

	void buildDependenceUnit(DAGNode *node)
	{
		auto opCode = node->getOpcode();
    if ( opCode == llvm::Instruction::Store )
		{
			// Add primaryOperand
			DAGNode *nodeSelector = node->getRight();
			const bool isOperator = true;
      llvm::outs() << "adding primaryOperand Node = " << node->getName() << "\n";
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
    else if ( node->getOpcode() == llvm::Instruction::Load || node->getOpcode() == llvm::Instruction::Store )
		{
			/// Collect Value of the load (node->right == nullptr)
			addDepWrapper(node->getLeft(), primaryOperand);
		}
		else
		{
      llvm::outs() << "adding binary operator Node = " << node->getName() << "\n";
			/// Binary operator case
			DepNode* Operator = addDep(node, primaryOperand, isOperator);

			addDepWrapper(node->getLeft(), Operator);
			addDepWrapper(node->getRight(), Operator);
		}
	}

	DepNode* addDep(DAGNode *node, DepNode* parentNode, bool isOperator)
	{
		/// Skip if not a pointer
		if ( node->getValue() != nullptr )
		{
			auto typeID = node->getValue()->getType()->getTypeID();
      if ( typeID != llvm::Type::PointerTyID )
				return nullptr;
		}

		DepNode *op;
		std::string name, opcodeName;
		opcodeName = node->getOpcodeName();

		/// If node is an operator collect the name, otherwise find the value node
		if (isOperator)
			name = node->getName();
		else
			name = findValueNode(node);

		/// Check whether that op is already present as a dependency
		if ( DG.isNamePresent(name) )
			op = DG.getDepNodeByName(name);
		else
			op = newDepNode(name, opcodeName, isOperator);

		if (parentNode == nullptr)
		{
			return op; // adding a parentNode
		}

		/// Add dependency to the parent node
		parentNode->addOp(op);
    llvm::outs() << "\tadding Node = " << node->getName() << " --> ";
    llvm::outs() << parentNode->getName() << "\n";
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
		DepNode* node = new DepNode(name, DG.getNodeCount(), opcodeName, isOperator);
		DG.setDepNode(node, name);
		return node;
	}

	// bool mergeVDG(DependencyGraph &VDG_source)
	// {
	// 	/// If source is empty, VDG remains unchanged
	// 	if ( VDG_source.empty() )
	// 		return true;

	// 	DepNode *root = VDG.findDepRoot();
	// 	DepNode *sourceRoot = VDG_source.findDepRoot();

	// 	DepNode *VDG_ptr, *VDG_source_ptr;
	// 	if ( !getFirstMatch(VDG_source, VDG_ptr, VDG_source_ptr) )
	// 		return false;

	// 	if ( VDG_source_ptr == sourceRoot )
	// 	{

	// 	}


	// }

	// bool getFirstMatch(DependencyGraph &VDG_source, DepNode *VDG_ptr, DepNode* VDG_source_ptr)
	// {
	// 	std::string name;
	// 	for (int id = 0; id < VDG_source.getNodeCount(); ++id)
	// 	{
	// 		name = VDG_source.getDepNodeName(id);
	// 		if ( VDG.isNamePresent(name) )
	// 		{
	// 			VDG_ptr = VDG.getDepNodeByName(name);
	// 			VDG_source_ptr = VDG_source.getDepNodeByName(name);
	// 			return true;
	// 		}
	// 	}
	// 	return false;
	// }
	

public:

	// void createVDG()
	// {
	// 	// assert( DG.isLocked() && 
	// 	// 		"Dependence graph(DG) has not been locked! Do so with lock()");
	// 	//traverse DG
	// 	// Recursively add, each op/successor
	// 	DG.markAllUnvisited();
	
	// 	DepNode* DGRoot = DG.findDepRoot();
	// 	if ( DGRoot->isAnOperator() )
	// 		DGRoot = findNextValueNode();
		
	// 	DepNode* VDGRoot = DGRoot;
	// 	VDG.setDepNode( VDGRoot, VDGRoot->getName() )

		 
	// }

	// DepNode* findNextValueNode(DepNode *node)
	// {
	// 	if ( !node->isAnOperator() && node->hasNotBeenVisited() )
	// 	{
	// 		node->markAsVisited();
	// 		return node;
	// 	}
	// 	else
	// 	{
	// 		auto successors = node->getOps();
	// 		for (auto successor=successors.begin() : successors)
	// 			findNextValueNode(successor);
	// 	}
	// }

	void printDG() 
	{
		DG.lock();
		DG.print();
		DG.unlock();
	}

	void printVDG()
	{
		VDG.lock();
		VDG.print();
		VDG.unlock();
	}

};


#endif /* VDG_BUILDER_H */
