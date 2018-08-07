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
	int edgeCount;

public:
	DepNode() : opCount(0), isOperator(false), isDependent(false), visited(false), edgeCount(0) {} 
	DepNode(std::string name, int id, std::string opcodeName, bool isOperator) : 
		name(name), ID(id), opCount(0), isOperator(isOperator), 
		opcodeName(opcodeName), isDependent(false), visited(false) {}
	DepNode(const DepNode &old)
	{
		name = old.name;
		ID = old.ID;
		opCount = old.opCount;
		isOperator = old.isOperator;
		opcodeName = old.opcodeName;
		DepNode* op;
		for (auto op_pair : old.Ops)
		{
			op = op_pair.second;
			Ops[op_pair.first] = new DepNode(*op);
		}
		opsKeyByName = old.opsKeyByName;
		isDependent = old.isDependent;
		visited = old.visited;
	}
	~DepNode() 
	{
		if ( hasDependents() )
		{
			for (auto op_pair : Ops)
				delete op_pair.second;
		}
	}

	bool hasNotBeenVisited() { return !visited; }
	bool isAnOperator() { return isOperator; }
	bool hasDependents() { return !Ops.empty(); }
	bool isADependent() { return isDependent; }
	std::string getName() { return name; }
	std::string getOpcodeName() { return opcodeName; }
	int getID() { return ID; }
	int getEdgeCount() { return edgeCount; }
	DepNodeList& getOps() { return Ops; } 
	DepNode* getDependent(const std::string &name) { return Ops[opsKeyByName[name]]; }

	void setID(int id) { ID = id; }
	void setName(std::string nm) { name = nm; }
	void markAsVisited() { visited = true; }
	void markAsUnvisited() { visited = false; }
	void setAsDependent() { isDependent = true; }
	void incrementEdgeCount() { edgeCount++; }
	void decrementEdgeCount() { edgeCount--; }
	
	void resetOps() 
	{
		opCount = 0;
		Ops.clear();
		opsKeyByName.clear(); 
	}

	void addOp(DepNode *opNode) 
	{ 
		if ( !isDependentPresent(opNode->getName()) )
		{
			int id = opNode->getID();
			Ops[opCount] = opNode; 
			Ops[opCount]->setID(id);
			Ops[opCount]->setName( opNode->getName() );
			opsKeyByName[opNode->getName()] = opCount;
			opNode->setAsDependent();
			opCount++;
		}
	}

	void updateOp(DepNode* opNode)
	{
		Ops[opsKeyByName[opNode->getName()]] = opNode;
	}

	void removeOps(bool operators=true)
	{
		if ( operators )
		{
			DepNode *op;
			for (auto op_pair : Ops)
			{
				op = op_pair.second;
				if ( op->isAnOperator() )
				{
					removeOp(op);
				}
			}
		}
		else
		{
			DepNode *op;
			for (auto op_pair : Ops)
			{
				op = op_pair.second;
				if ( !op->isAnOperator() )
				{
					removeOp(op);
				}
			}
		}	
	}

	void removeOp(DepNode *opNode)
	{
		opCount--;
		Ops.erase( opsKeyByName[opNode->getName()] );
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
		outs() << "\t    Name: ";
		if (isOperator)
			outs() << opcodeName << " ";
		outs() << name;

		if ( hasDependents() )
		{
			outs() << "\n\t Members: ";
			for (auto op_pair : Ops)
				outs() << "DepNode" << op_pair.second->getID() << " ";
		}
		outs() << "\n";
	}

};

#endif /* DEPENDENCY_NODE_H */ 
