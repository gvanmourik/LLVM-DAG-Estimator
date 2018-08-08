#ifndef DEPENDENCY_NODE_H
#define DEPENDENCY_NODE_H

#include <map>
#include <llvm/IR/Instruction.h>

class DepNode;
typedef std::map<int, DepNode*> DepNodeList;
typedef std::unordered_map<int, DepNode*> DepVertexList;
typedef std::unordered_map<std::string, int> DepNameList;

class DepNode {

private:
	std::string name;
	int ID;
	int successorCount;
	bool isOperator;
	std::string opcodeName;
	DepNodeList Successors;
	DepNameList keyByName;
	bool isSuccessor;
	bool visited;
	int edgeCount;

public:
	DepNode() : successorCount(0), isOperator(false), isSuccessor(false), visited(false), edgeCount(0) {} 
	DepNode(std::string name, int id, std::string opcodeName, bool isOperator) : 
		name(name), ID(id), successorCount(0), isOperator(isOperator), 
		opcodeName(opcodeName), isSuccessor(false), visited(false) {}
	DepNode(const DepNode &old)
	{
		name = old.name;
		ID = old.ID;
		successorCount = old.successorCount;
		isOperator = old.isOperator;
		opcodeName = old.opcodeName;
		DepNode* successor;
		for (auto successor_pair : old.Successors)
		{
			successor = successor_pair.second;
			Successors[successor_pair.first] = new DepNode(*successor);
		}
		keyByName = old.keyByName;
		isSuccessor = old.isSuccessor;
		visited = old.visited;
	}
	~DepNode() 
	{
		if ( hasSuccessors() )
		{
			for (auto successor_pair : Successors)
				delete successor_pair.second;
		}
	}

	bool hasNotBeenVisited() { return !visited; }
	bool isAnOperator() { return isOperator; }
	bool hasSuccessors() { return !Successors.empty(); }
	bool isASuccessor() { return isSuccessor; }
	std::string getName() { return name; }
	std::string getOpcodeName() { return opcodeName; }
	int getID() { return ID; }
	int getEdgeCount() { return edgeCount; }
	DepNodeList& getSuccessors() { return Successors; } 
	DepNode* getSuccessor(const std::string &name) { return Successors[keyByName[name]]; }

	void setID(int id) { ID = id; }
	void setName(std::string nm) { name = nm; }
	void markAsVisited() { visited = true; }
	void markAsUnvisited() { visited = false; }
	void setAsSuccessor() { isSuccessor = true; }
	void incrementEdgeCount() { edgeCount++; }
	void decrementEdgeCount() { edgeCount--; }
	
	void resetSuccessors() 
	{
		successorCount = 0;
		Successors.clear();
		keyByName.clear(); 
	}

	void addSuccessor(DepNode *opNode) 
	{ 
		if ( !isSuccessorPresent(opNode->getName()) )
		{
			int id = opNode->getID();
			Successors[successorCount] = opNode; 
			Successors[successorCount]->setID(id);
			Successors[successorCount]->setName( opNode->getName() );
			keyByName[opNode->getName()] = successorCount;
			opNode->setAsSuccessor();
			successorCount++;
		}
	}

	void updateSuccessors(DepNode* opNode)
	{
		if (opNode != nullptr)
			Successors[keyByName[opNode->getName()]] = opNode;
	}

	void removeSuccessors(bool operators=true)
	{
		if ( operators )
		{
			DepNode *successor;
			for (auto successor_pair : Successors)
			{
				successor = successor_pair.second;
				if ( successor->isAnOperator() )
				{
					removeSuccessor(successor);
				}
			}
		}
		else
		{
			DepNode *successor;
			outs() << "Successors.size() = " << Successors.size() << "\n";
			for (int i=0; i<(int)Successors.size(); ++i)
			{
				outs() << "beginning of for...\n";
				// successor = successor_pair->second;
				successor = Successors[i];
				// outs() << "successor = " << successor->getName() << "\n";
				// successor->print();
				if ( !successor->isAnOperator() )
				{
					if ( successor != nullptr )
						removeSuccessor(successor);
				}
			}
		}	
	}

	void removeSuccessor(DepNode *opNode)
	{
		outs() << "Removing '" << opNode->getName() << "' from " << name << "\n";
		successorCount--;
		Successors.erase( keyByName[opNode->getName()] );
		outs() << "after erase...\n";
	}

	bool isSuccessorPresent(const std::string &name)
	{
		if( keyByName.count(name) == 0 )
			return false;
		else
			return true;
	}

	void print()
	{
		outs() << "\t    Name: ";
		if (isOperator)
			outs() << opcodeName << " ";
		outs() << name;

		if ( hasSuccessors() )
		{
			outs() << "\n\t Members: ";
			for (auto successor_pair : Successors)
				outs() << "DepNode" << successor_pair.second->getID() << " ";
		}
		outs() << "\n";
	}

};

#endif /* DEPENDENCY_NODE_H */ 
