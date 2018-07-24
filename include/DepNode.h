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
	bool isDependent;
	// DepNameList opsKeyByName;

public:
	DepNode() : opCount(0), isOperator(false), isDependent(false) {}
	DepNode(std::string name, int id, std::string opcodeName, bool isOperator) : 
		name(name), ID(id), opCount(0), isOperator(isOperator), 
		opcodeName(opcodeName), isDependent(false) {}
	~DepNode() 
	{
		for (auto iter=Ops.begin(); iter!=Ops.end(); ++iter)
			delete iter->second;
	}

	bool isAnOperator() { return isOperator; }
	bool hasDependents() { return !Ops.empty(); }
	bool isADependent() { return isDependent; }
	std::string getName() { return name; }
	std::string getOpcodeName() { return opcodeName; }
	int getID() { return ID; }
	DepNodeList getOps() { return Ops; }

	void setID(int id) { ID = id; }
	void setAsDependent() { isDependent = true; }

	void addOp(DepNode *opNode, int id) 
	{ 
		Ops[opCount] = opNode; 
		Ops[opCount]->setID(id);
		opNode->setAsDependent();
		opCount++;
	}

	void print()
	{
		outs() << "\t    Name: ";
		if (isOperator)
		{
			outs() << opcodeName << " ";
			// outs() << " (ID: " << ID << ")";
		}	
		outs() << name;

		if ( hasDependents() )
		{
			outs() << "\n\t Members: ";
			for (auto iter=Ops.begin(); iter!=Ops.end(); ++iter)
				outs() << "DepNode" << iter->second->getID() << " ";
		}
		outs() << "\n";
	}

};

#endif /* DEPENDENCY_NODE_H */ 
