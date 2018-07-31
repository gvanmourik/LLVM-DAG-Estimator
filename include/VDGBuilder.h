#ifndef VDG_BUILDER_H
#define VDG_BUILDER_H

#include <queue>
#include "DepNode.h"
#include "DAGNode.h"


class VariableDependencyGraph 
{
friend class VDGBuilder;
private:
	DepVertexList DependenceVertices;
	DepNameList keyByName;
	int ID;
	bool VDGIsLocked;

public:
	VariableDependencyGraph(): ID(0), VDGIsLocked(false) {}
	~VariableDependencyGraph() {}

	void unlock() { VDGIsLocked = false; }
	void lock() { VDGIsLocked = true; }
	
	bool isVDGLocked() { return VDGIsLocked; }
	int getDepID() { return ID; }
	int getWidth() { return findWidth(); }
	int getDepth() { return findDepth(); }
	DepNode* getDepNode(int ID) { return DependenceVertices[ID]; }
	DepNode* getDepNodeByName(std::string name) { return DependenceVertices[keyByName[name]]; }

protected:
	void setDepNode(DepNode* node, std::string name) 
	{
		DependenceVertices[ID] = node;
		keyByName[name] = ID;
		ID++;
	}

	bool isNamePresentDep(std::string name)
	{
		if( keyByName.count(name) == 0 )
			return false;
		else
			return true;
	}

private:
	/// TRAVERSAL
	int findWidth()
	{
		assert( VDGIsLocked && 
				"Variable dependence graph(VDG) has not been locked! Do so with lock()");
		DepNode *root = findDepRoot();
		if ( root == nullptr )
			return 0;

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
					if ( iter->second->hasNotBeenVisited() )
					{
						wavefront.push(iter->second);
						iter->second->markAsVisited();
					}
				}
			}
		}
		return maxWidth;
	}

	int findDepth()
	{
		assert( VDGIsLocked && 
				"Variable dependence graph(VDG) has not been locked! Do so with lock()");
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

public:
	void clear() 
	{ 
		DependenceVertices.clear();
		keyByName.clear();  
	}

	void print()
	{
		assert( VDGIsLocked && 
				"Variable dependence graph(VDG) has not been locked! Do so with lock()");
		outs() << "\nDependency Nodes:\n";
		for (int i = 0; i < ID; ++i)
		{
			outs() << "---------------------------------------------------\n";
			outs() << "DepNode" << i << ":\n";
			DependenceVertices[i]->print();
			outs() << "---------------------------------------------------\n";
		}
		outs() << "\n";
	}
};


class VDGBuilder
{
private:
	VariableDependencyGraph VDG;
	DAGVertexList DAGVertices;
	const int DAGNodeCount;


public:
	VDGBuilder(DAGVertexList &DAGVertices, int DAGNodeCount) :
		DAGVertices(DAGVertices), DAGNodeCount(DAGNodeCount) {}
	~VDGBuilder() {}

	void clearVDG() { VDG.clear(); }
	void createVDG()
	{
		for (int i = 0; i < DAGNodeCount; ++i)
		{
			traverseDAGNode(DAGVertices[i]);
		}
	}

	VariableDependencyGraph getVDG() 
	{
		VDG.lock();
		auto VDG_copy = VDG;
		VDG.unlock();
		return VDG_copy;
	}

	void mergeVDG(VariableDependencyGraph &VDG_source)
	{

	}


private:
	void traverseDAGNode(DAGNode *node)
	{
		if ( node != nullptr )
		{	
			auto opCode = node->getOpcode();
			if ( opCode == Instruction::Store )
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

	void printVDG() 
	{
		VDG.lock();
		VDG.print();
		VDG.unlock();
	}

};


#endif /* VDG_BUILDER_H */