#ifndef VARIABLE_DEP_GRAPH_H
#define VARIABLE_DEP_GRAPH_H

#include <queue>
#include "DepNode.h"

class VariableDependencyGraph 
{
friend class DAGBuilder;
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


#endif /* VARIABLE_DEP_GRAPH_H */