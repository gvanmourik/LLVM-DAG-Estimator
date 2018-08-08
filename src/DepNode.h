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
	int opCount;
	bool isOperator;
	std::string opcodeName;
	DepNodeList Ops;
	DepNameList opsKeyByName;
	bool isDependent;
	bool visited;

public:
	DepNode() : opCount(0), isOperator(false), isDependent(false), visited(false) {}
	DepNode(std::string name, int id, std::string opcodeName, bool isOperator) : 
		name(name), ID(id), opCount(0), isOperator(isOperator), 
		opcodeName(opcodeName), isDependent(false), visited(false) {}
	~DepNode() 
	{
		for (auto iter=Ops.begin(); iter!=Ops.end(); ++iter)
			delete iter->second;
	}

	bool hasNotBeenVisited() { return !visited; }
	bool isAnOperator() { return isOperator; }
	bool hasDependents() { return !Ops.empty(); }
	bool isADependent() { return isDependent; }
	std::string getName() { return name; }
	std::string getOpcodeName() { return opcodeName; }
	int getID() { return ID; }
	DepNodeList getOps() { return Ops; }
	DepNode* getDependent(const std::string &name) { return Ops[opsKeyByName[name]]; }

	void setID(int id) { ID = id; }
	void markAsVisited() { visited = true; }
	void setAsDependent() { isDependent = true; }
	// void markAllUnvisited()
	// {
	// 	for (auto op=Ops.begin() : Ops)
	// 		op.visited = false;
	// }

	void addOp(DepNode *opNode) 
	{ 
		if ( !isDependentPresent(opNode->getName()) )
		{
			int id = opNode->getID();
			Ops[opCount] = opNode; 
			Ops[opCount]->setID(id);
			opsKeyByName[opNode->getName()] = id;
			opNode->setAsDependent();
			opCount++;
		}
	}

	bool isDependentPresent(const std::string &name)
	{
		if( opsKeyByName.count(name) == 0 )
			return false;
		else
			return true;
	}

	void print()
	{
    llvm::outs() << "\t    Name: ";
		if (isOperator)
		{
      llvm::outs() << opcodeName << " ";
			// outs() << " (ID: " << ID << ")";
		}	
    llvm::outs() << name;

		if ( hasDependents() )
		{
      llvm::outs() << "\n\t Members: ";
			for (auto iter=Ops.begin(); iter!=Ops.end(); ++iter)
        llvm::outs() << "DepNode" << iter->second->getID() << " ";
		}
    llvm::outs() << "\n";
	}

};

#endif /* DEPENDENCY_NODE_H */ 
