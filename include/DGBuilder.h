#ifndef DG_BUILDER_H
#define DG_BUILDER_H

#include "DependencyGraph.h"


class DGBuilder
{
private:
	DependencyGraph DG;
	DependencyGraph VDG; //variable dependence graph
	DependencyGraph ODG; //operator dependence graph
	DAGVertexList DAGVertices;
	const int DAGNodeCount;


public:
	DGBuilder(DAGVertexList &DAGVertices, int DAGNodeCount) :
		DAGVertices(DAGVertices), DAGNodeCount(DAGNodeCount) {}
	~DGBuilder() {}

	void clearDG() { DG.clear(); }
	void createDG()
	{
		// assert( !DG.isLocked() && 
		// 		"Dependence graph(DG) is locked! No changes can be made until it is unlocked.");
		for (int i = 0; i < DAGNodeCount; ++i)
		{
			traverseDAGNode(DAGVertices[i]);
		}
	}

	int getVDGWidth() 
	{
		VDG.lock();
		auto width = VDG.findWidth();
		VDG.unlock();
		return width;
	}
	int getVDGDepth() 
	{
		VDG.lock();
		auto depth = VDG.findDepth();
		VDG.unlock();
		return depth;
	}
	int getODGWidth() 
	{
		ODG.lock();
		auto width = ODG.findWidth();
		ODG.unlock();
		return width;
	}
	int getODGDepth() 
	{
		ODG.lock();
		auto depth = ODG.findDepth();
		ODG.unlock();
		return depth;
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
		else if ( node->getOpcode() == Instruction::Load || node->getOpcode() == Instruction::Store )
		{
			/// Collect Value of the load (node->right == nullptr)
			addDepWrapper(node->getLeft(), primaryOperand);
		}
		else
		{
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
			if ( typeID != Type::PointerTyID )
				return nullptr;
		}

		DepNode *successor;
		std::string name, opcodeName;
		opcodeName = node->getOpcodeName();

		/// If node is an operator collect the name, otherwise find the value node
		if (isOperator)
			name = node->getName();
		else
			name = findValueNode(node);

		/// Check whether that successor is already present as a dependency
		if ( DG.isNamePresent(name) )
			successor = DG.getDepNodeByName(name);
		else
			successor = newDepNode(name, opcodeName, isOperator);

		if (parentNode == nullptr)
		{
			return successor; // adding a parentNode
		}

		/// Add dependency to the parent node
		parentNode->addSuccessor(successor);
		return successor; // adding a member
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
		DG.addDepNode(node, name);
		return node;
	}
	

public:

	void createVDG()
	{	
		bool removeOperators = true;
		VDG = DG;
		VDG.setName("VDG");
		VDG.removeEdges(removeOperators);
		VDG.updateEdgeCount();
	}

	void createODG()
	{
		bool removeOperators = false;
		ODG = DG;
		ODG.setName("ODG");
		ODG.removeEdges(removeOperators);
		ODG.updateEdgeCount();
	}

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


#endif /* DG_BUILDER_H */