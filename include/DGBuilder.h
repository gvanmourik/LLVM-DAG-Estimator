#ifndef VDG_BUILDER_H
#define VDG_BUILDER_H

#include <queue>
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
	std::string graphName;

public:
	DependencyGraph(): ID(0), DGIsLocked(false) {}
	DependencyGraph(const DependencyGraph &graph)
	{
		// DependenceVertices = graph.DependenceVertices;
		DepNode* node;
		for (auto vertex_pair : graph.DependenceVertices)
		{
			node = vertex_pair.second;
			if ( node != nullptr )
				DependenceVertices[vertex_pair.first] = new DepNode(*node);
		}
		keyByName = graph.keyByName;
		ID = graph.ID;
		DGIsLocked = graph.DGIsLocked;
	}
	~DependencyGraph() {}

	DependencyGraph& operator=(const DependencyGraph &graph)
	{
		// DependenceVertices = graph.DependenceVertices;
		DepNode* node;
		for (auto vertex_pair : graph.DependenceVertices)
		{
			node = vertex_pair.second;
			DependenceVertices[vertex_pair.first] = new DepNode(*node);
		}
		keyByName = graph.keyByName;
		ID = graph.ID;
		DGIsLocked = graph.DGIsLocked;
		return *this;
	}

	void unlock() { DGIsLocked = false; }
	void lock() { DGIsLocked = true; }
	
	bool isLocked() { return DGIsLocked; }
	int getNodeCount() { return ID; }
	int getWidth() { return findWidth(); }
	int getDepth() { return findDepth(); }
	std::string getDepNodeName(int ID) { return DependenceVertices[ID]->getName(); }
	DepNode* getDepNode(int ID) { return DependenceVertices[ID]; }
	DepNode* getDepNodeByName(std::string name) { return DependenceVertices[keyByName[name]]; }
	DepVertexList getDGVertices() { return DependenceVertices; }
	std::string getName() { return graphName; }

	void setDGVertices(DepVertexList vertices) { DependenceVertices = vertices; }
	void setName(std::string name) { graphName = name; }

protected:
	void addDepNode(DepNode* node, std::string name) 
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

	void updateEdgeCount()
	{
		DepNode *vertex;
		std::string name;
		for (auto vertex_pair : DependenceVertices)
		{
			vertex = vertex_pair.second;
			name = vertex->getName();

			for (auto vertex_pair_inner : DependenceVertices)
			{
				if ( vertex_pair_inner.second->isDependentPresent(name) )
					vertex->incrementEdgeCount();
			}
		}
	}

	DepNode* findDepRoot()
	{
		DepNode *currentNode;
		for (int i = 0; i < ID; ++i)
		{
			currentNode = DependenceVertices[i];
			if ( currentNode != nullptr )
			{
				if ( currentNode->hasDependents() && !currentNode->isADependent() )
					return currentNode;
			}
		}
		return nullptr;
	}

	void markAllUnvisited()
	{
		for (auto vertex : DependenceVertices)
		{
			if ( vertex.second != nullptr )
				vertex.second->markAsUnvisited();
		}
	}

	/// TRAVERSAL
	int findWidth()
	{
		assert( DGIsLocked && 
				"Dependence graph(DG) has not been locked! Do so with lock()");
		markAllUnvisited();

		DepNode *root = findDepRoot();
		if ( root == nullptr )
			return 0;

		std::queue<DepNode*> wavefront;
		wavefront.push(root);

		int maxWidth = 1; // to avoid 1st assignment in while
		int width = 0;
		DepNode *selector;
		while ( !wavefront.empty() )
		{
			width = wavefront.size();
			if ( maxWidth < width )
				maxWidth = width;

			selector = wavefront.front();
			wavefront.pop();

			if ( selector->hasDependents() )
			{
				auto ops = selector->getOps();

				DepNode* op;
				for (auto op_pair : ops)
				{
					op = op_pair.second;
					if ( op->getEdgeCount() == 1 )
					{
						wavefront.push(op);
						op->markAsVisited();
						op->decrementEdgeCount();
					}
					if ( op->getEdgeCount() > 1 )
						op->decrementEdgeCount();
				}
			}
		}
		return maxWidth;
	}

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

	void removeEdges(bool operatorEdges=true, bool variableEdges=false)
	{
		DepNode *vertex;
		for (auto vertex_pair : DependenceVertices)
		{
			vertex = vertex_pair.second;
			removeSuccessorEdges(vertex, vertex, operatorEdges, variableEdges);
		}
		cleanUpVertices(operatorEdges, variableEdges);
	}

	void removeSuccessorEdges(DepNode* parentNode, DepNode* opNode, bool operatorEdges=true, bool variableEdges=false)
	{
		auto ops = opNode->getOps();
		DepNode *op;
		for (auto op_pair : ops)
		{
			op = op_pair.second;

			if ( operatorEdges )
			{
				if ( !op->isAnOperator() )
				{
					if ( !parentNode->isAnOperator() )
						parentNode->addOp(op);
				}
				else
					removeSuccessorEdges(parentNode, op, operatorEdges, variableEdges);
			}
			// if ( variableEdges )
			// {
			// 	if ( !op->isAnOperator() )
			// 		removeSuccessorEdges(op, parentNode, operatorEdges, variableEdges);
			// 	else
			// 		parentNode->addOp(op);
			// }
		}
	}

	void cleanUpVertices(bool operatorEdges=true, bool variableEdges=false)
	{
		DepNode *vertex;
		for (int key=0; key<ID; ++key)
		{
			vertex = DependenceVertices[key];
			if ( vertex != nullptr )
			{
				if ( operatorEdges )
				{
					if ( vertex->isAnOperator() )
					{
						auto name = vertex->getName();
						vertex->resetOps();
						DependenceVertices.erase(key);
					}
					else
					{
						// remove pointers to operators
						vertex->removeOps(operatorEdges);

						// update op pointers
						auto ops = vertex->getOps();
						DepNode *op, *updatedOp;
						for (auto op_pair : ops)
						{
							op = op_pair.second;
							updatedOp = DependenceVertices[keyByName[op->getName()]];
							vertex->updateOp(updatedOp);
						}
					}
				}
				// if ( variableEdges )
				// {
				// 	if ( !vertex->isAnOperator() )
				// 		DependenceVertices.erase(key);
				// }
			}
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
		outs() << "\nDependency Nodes for " << graphName << ":\n";
		for (auto vertex_pair : DependenceVertices)
		{
			if ( vertex_pair.second != nullptr )
			{
				outs() << "---------------------------------------------------\n";
				outs() << "DepNode" << vertex_pair.second->getID() << ":\n";
				vertex_pair.second->print();
				outs() << "---------------------------------------------------\n";
			}
		}
		outs() << "\n";
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
		DG.addDepNode(node, name);
		return node;
	}
	

public:

	void createVDG()
	{	
		VDG = DG;
		VDG.setName("VDG");
		VDG.removeEdges();
		VDG.updateEdgeCount();
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


#endif /* VDG_BUILDER_H */