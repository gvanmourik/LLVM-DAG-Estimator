#ifndef DEPENDENCY_GRAPH_H
#define DEPENDENCY_GRAPH_H

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
			if (vertex != nullptr)
			{
				name = vertex->getName();

				for (auto vertex_pair_inner : DependenceVertices)
				{
					if ( vertex_pair_inner.second->isSuccessorPresent(name) )
						vertex->incrementEdgeCount();
				}
			}
		}
	}

	DepNode* findRoot()
	{
		DepNode *currentNode;
		for (int i = 0; i < ID; ++i)
		{
			currentNode = DependenceVertices[i];
			if ( currentNode != nullptr )
			{
				if ( currentNode->hasSuccessors() && !currentNode->isASuccessor() )
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
		DepNode *root = findRoot();
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

			if ( selector->hasSuccessors() )
			{
				auto ops = selector->getSuccessors();

				DepNode* successor;
				for (auto successor_pair : ops)
				{
					successor = successor_pair.second;
					if ( successor->getEdgeCount() == 1 )
					{
						wavefront.push(successor);
						successor->decrementEdgeCount();
					}
					if ( successor->getEdgeCount() > 1 )
						successor->decrementEdgeCount();
				}
			}
		}
		return maxWidth;
	}

	int findDepth()
	{
		assert( DGIsLocked && 
				"Dependence graph(DG) has not been locked! Do so with lock()");
		DepNode *root = findRoot();
		if ( root == nullptr )
			return 0;

		int maxDepth = 0;
		int depthCount = 1;
		depthWrapper(root, depthCount, maxDepth);

		return maxDepth;
	}

	void depthWrapper(DepNode *node, int count, int &maxCount)
	{
		auto Ops = node->getSuccessors();
		if ( !node->isAnOperator() )
			count++;

		for (auto iter=Ops.begin(); iter!=Ops.end(); ++iter)
		{
			if ( maxCount < count )
				maxCount = count;
			depthWrapper(iter->second, count, maxCount);
		}
	}

	/// Used in forming either a VDG or ODG
	void removeEdges(bool operatorEdges=true)
	{
		DepNode *vertex;
		for (auto vertex_pair : DependenceVertices)
		{
			vertex = vertex_pair.second;
			removeSuccessorEdges(vertex, vertex, operatorEdges);
		}
		cleanUpVertices(operatorEdges);
	}

	void removeSuccessorEdges(DepNode* parentNode, DepNode* opNode, bool operatorEdges=true)
	{
		auto ops = opNode->getSuccessors();
		DepNode *successor;
		for (auto successor_pair : ops)
		{
			successor = successor_pair.second;

			if ( operatorEdges )
			{
				if ( !successor->isAnOperator() )
				{
					if ( !parentNode->isAnOperator() )
						parentNode->addSuccessor(successor);
				}
				else
					removeSuccessorEdges(parentNode, successor, operatorEdges);
			}
			else
			{
				if ( successor->isAnOperator() )
				{
					if ( !parentNode->isAnOperator() )
						parentNode->addSuccessor(successor);
				}
				else
					removeSuccessorEdges(parentNode, successor, operatorEdges);
			}
		}
	}

	void cleanUpVertices(bool operatorEdges=true)
	{
		DepNode *vertex;
		for (int key=0; key<ID; ++key)
		{
			vertex = DependenceVertices[key];
			if ( operatorEdges )
			{
				if ( vertex->isAnOperator() )
				{
					auto name = vertex->getName();
					vertex->resetSuccessors();
					DependenceVertices.erase(key);
				}
				else
				{
					// remove pointers to operators
					vertex->removeSuccessors(operatorEdges);

					// update successor pointers
					auto ops = vertex->getSuccessors();
					DepNode *successor, *updatedSuccessor;
					for (auto successor_pair : ops)
					{
						successor = successor_pair.second;
						updatedSuccessor = DependenceVertices[keyByName[successor->getName()]];
						vertex->updateSuccessors(updatedSuccessor);
					}
				}
			}
			else
			{
				if ( !vertex->isAnOperator() )
				{
					auto name = vertex->getName();
					vertex->resetSuccessors();
					DependenceVertices.erase(key);
				}
				else
				{
					// remove pointers to operators
					// outs() << "--->vertex = " << vertex->getName() << "\n";
					vertex->print();
					vertex->removeSuccessors(operatorEdges);

					// update successor pointers
					auto successors = vertex->getSuccessors();
					DepNode *successor, *updatedSuccessor;
					for (auto successor_pair : successors)
					{
						successor = successor_pair.second;
						updatedSuccessor = DependenceVertices[keyByName[successor->getName()]];
						vertex->updateSuccessors(updatedSuccessor);
					}
				}
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
		llvm::outs() << "\nDependency Nodes for " << graphName << ":\n";
		for (auto vertex_pair : DependenceVertices)
		{
			if ( vertex_pair.second != nullptr )
			{
				llvm::outs() << "---------------------------------------------------\n";
				llvm::outs() << "DepNode" << vertex_pair.second->getID() << ":\n";
				vertex_pair.second->print();
				llvm::outs() << "---------------------------------------------------\n";
			}
		}
		llvm::outs() << "\n";
	}
};


#endif /* DEPENDENCY_GRAPH_H */