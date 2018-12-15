#ifndef DAG_NODE_H
#define DAG_NODE_H

#include <unordered_map>
#include <llvm/IR/Instruction.h>

class DAGNode;
typedef enum vertex_t{VAL, INST, FUNC} vertex_t;
typedef std::unordered_map<DAGNode*, DAGNode*> DAGVertexList;
typedef std::unordered_map<llvm::Value*, DAGNode*> DAGValueList;

class DAGNode {

private:
	int varWidth;
	int varDepth;
	int opWidth;
	int opDepth;
	DAGNode* valueNode;
	llvm::Value* llvmValue; //holds either instruction or value
	vertex_t type;
	std::string constName;
	DAGVertexList Successors;


public:
	DAGNode() : varWidth(0), varDepth(0), opWidth(0), opDepth(0), valueNode(nullptr), type(VAL) {}
	DAGNode(llvm::Value* value, vertex_t type) : varWidth(0), varDepth(0), 
		opWidth(0), opDepth(0), valueNode(nullptr), llvmValue(value), type(type) {}
	~DAGNode() {}

	int getVarWidth() { return varWidth; }
	int getVarDepth() { return varDepth; }
	int getOpWidth() { return opWidth; }
	int getOpDepth() { return opDepth; }
	vertex_t getType() { return type; }
	llvm::Value* getllvmValue() { return llvmValue; }
	llvm::Type* getllvmValueTy() { return llvmValue->getType(); }
	std::string getConstName() { return constName; }
	DAGVertexList getSuccessors() { return Successors; }
	DAGNode* getValueNode() { return valueNode; }
	bool hasSuccessors() { return !Successors.empty(); }

	void setVarWidth(int width) { varWidth = width; }
	void setVarDepth(int depth) { varDepth = depth; }
	void setOpWidth(int width) { opWidth = width; }
	void setOpDepth(int depth) { opDepth = depth; }
	void setValueNode(DAGNode* node) { valueNode = node; }
	void setConstName(std::string name) { constName = name; }

	/// Best to first check if successor is present to avoid returning null
	DAGNode* getSuccessor(DAGNode* successor)
	{
		if ( isSuccessorPresent(successor) )
			return Successors[successor];
		else
			return nullptr;
	}

	std::string getName() 
	{
		if ( !constName.empty() )
			return constName;
		else
		{
			if ( llvmValue->hasName() )
				return llvmValue->getName();
			else
			{
				llvmValue->dump();
				return "<no name>"; 
			}
		}
	}

	bool setName(std::string name)
	{
		if ( llvmValue->hasName() )
		{
			return false;
		}
		llvmValue->setName(name);
		return true;
	}

	bool addSuccessor(DAGNode* successor)
	{
		if ( successor != nullptr || !isSuccessorPresent(successor) ) 
		{
			Successors[successor] = successor;
			return true;
		}
		return false;
	}

	bool removeSuccessor(DAGNode* successor)
	{
		if ( successor != nullptr || isSuccessorPresent(successor) ) 
		{
			Successors.erase(successor);
			return true;
		}
		return false;
	}

	bool hasStoredValue()
	{
		DAGNode* successor;
		for (auto successor_pair : Successors)
		{
			successor = successor_pair.second;
			auto inst = successor->getllvmValue();
			if ( successor->getType() == INST && 
				 llvm::isa<llvm::StoreInst>(inst) )
			{
				return true;
			}
		}
		return false;
	}

	DAGNode* getStoredValueNode()
	{
		DAGNode* successor;
		DAGNode* storedValueNode;
		for (auto successor_pair : Successors)
		{
			successor = successor_pair.second;
			auto inst = successor->getllvmValue();
			if ( successor->getType() == INST && 
				 llvm::isa<llvm::StoreInst>(inst) )
			{
				storedValueNode = successor;
			}
		}
		return storedValueNode;
	}

	// void removeSuccessors() { Successors.clear(); }

	bool isSuccessorPresent(DAGNode* successor)
	{
		if (Successors.count(successor) != 0)
			return true;
		else
			return false;
	}

	bool isaInst()
	{
		if (type == INST)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void print()
	{
		if (type == INST)
		{
			auto inst = llvm::cast<llvm::Instruction>(llvmValue);
      		llvm::outs() << "\t  Instruction: " << inst->getOpcodeName();
      		llvm::outs() << " (" << getName() << ")";
      		llvm::outs() << "\n\t\t\t(Variable Width: " << varWidth << ")";
      		llvm::outs() << " (Operator Width: " << opWidth << ")\n";
      		llvm::outs() << "\t\t\t(ID: " << &*llvmValue << ")\n";
		}
		else if (type == FUNC)
		{
			llvm::outs() << "\tFunction Call: " << getName();
			llvm::outs() << " (ID: " << &*llvmValue << ")\n";
		}
		else
		{
      		llvm::outs() << "\t        Value: " << getName();
      		llvm::outs() << " (ID: " << &*llvmValue << ")\n";
      	}	
    	
		
		/// Recursively print each successor
		DAGNode* successor;
		for (auto successor_pair : Successors)
		{
			successor = successor_pair.second;
			successor->print();
		}
	}
	
	void printSuccessorsNames()
	{
		for (auto successor_pair : Successors)
		{
      		llvm::outs() << successor_pair.second->getName() << " ";
		}
    	llvm::outs() << "\n";
	}

};


#endif /* DAG_NODE_H */ 
