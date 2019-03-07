#ifndef DAG_NODE_H
#define DAG_NODE_H

#include <cstdint>
#include <sstream>
#include <unordered_map>
#include <llvm/IR/Instruction.h>

class DAGNode;
typedef enum vertex_t{VAL, INST, FUNC} vertex_t;
typedef enum adjNode_t{IN, OUT} adjNode_t;
typedef std::unordered_map<DAGNode*, DAGNode*> DAGVertexList;
typedef std::unordered_map<llvm::Value*, DAGNode*> DAGValueList;

class DAGNode {

private:
	int varWidth;
	int varDepth;
	int opWidth;
	int opDepth;
	bool visited;
	DAGNode* valueNode;
	llvm::Value* llvmValue; //holds either inst or value
	vertex_t type;
	std::string constName;
	DAGVertexList Predecessors;
	DAGVertexList Successors;


public:
	DAGNode() : varWidth(0), varDepth(0), opWidth(0), opDepth(0), visited(false), valueNode(nullptr), type(VAL) {}
	DAGNode(llvm::Value* value, vertex_t type) : varWidth(0), varDepth(0), 
		opWidth(0), opDepth(0), visited(false), valueNode(nullptr), llvmValue(value), type(type) {}
	~DAGNode() {}

	int getVarWidth() { return varWidth; }
	int getVarDepth() { return varDepth; }
	int getOpWidth() { return opWidth; }
	int getOpDepth() { return opDepth; }
	vertex_t getType() { return type; }
	llvm::Value* getllvmValue() { return llvmValue; }
	llvm::Type* getllvmValueTy() { return llvmValue->getType(); }
	std::string getConstName() { return constName; }
	DAGNode* getValueNode() { return valueNode; }
	DAGVertexList& getSuccessors() { return Successors; }
	bool hasSuccessors() { return !Successors.empty(); }
	// DAGVertexList Predecessors() { return Predecessors; }
	bool hasPredecessors() { return !Predecessors.empty(); }

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

	bool addPredecessor(DAGNode* predecessor)
	{
		if ( predecessor != nullptr || !isPredecessorPresent(predecessor) ) 
		{
			Predecessors[predecessor] = predecessor;
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

	void setVisited(bool status)
	{
		visited = status;
	}

	void setAllUnvisited()
	{
		visited = false;
		DAGNode* successor;
		for (auto successor_pair : Successors)
		{
			successor = successor_pair.second;
			successor->setVisited(false);
		}
	}

	bool hasBeenVisited()
	{
		if (visited == true)
			return true;
		else
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

	bool isPredecessorPresent(DAGNode* predecessor)
	{
		if (Predecessors.count(predecessor) != 0)
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
    	
		
		/// Iteratively print each successor
		DAGNode* successor;
		for (auto successor_pair : Successors)
		{
			successor = successor_pair.second;
			// to prevent an infinite loop mark as visited
			if ( !successor->hasBeenVisited() )
			{
				successor->setVisited(true);
				successor->print();
			}
		}
		setAllUnvisited(); //reset visited status for next print call
	}

	void prettyPrint(int tabCountLeft=0, int tabCountRight=2, int biOpCount=0)
	{
		if (type == INST && llvm::isa<llvm::BinaryOperator>(llvmValue))
		{
			biOpCount++;

			auto inst = llvm::cast<llvm::Instruction>(llvmValue);
			//vertical split
			llvm::outs() << generateTabs(tabCountLeft) << 
				" (" << inst->getOpcodeName() << " " << getName() << ")_____ \n";
			llvm::outs() << generateTabs(tabCountLeft) << " |" << generateTabs(tabCountRight) << "\\ \n";
			llvm::outs() << generateTabs(tabCountLeft) << " |" << generateTabs(tabCountRight) << " \\\n";

			
		}
		else
		{
			llvm::outs() << generateTabs(tabCountLeft) << " (" << getName() << ") \n";
			if ( biOpCount > 0 && tabCountLeft == 0)
			{
				if ( hasSuccessors() )
				{
					llvm::outs() << generateTabs(tabCountLeft) << " |" << generateTabs(tabCountRight) << " |\n";
					llvm::outs() << generateTabs(tabCountLeft) << " |" << generateTabs(tabCountRight) << " |\n";
				}	
			}
			else
			{
				if ( hasSuccessors() )
				{
					llvm::outs() << generateTabs(tabCountLeft) << " |\n";
					llvm::outs() << generateTabs(tabCountLeft) << " |\n";
				}
			}	
		}

		int successorCount = 0;
		DAGNode* successor;
		for (auto successor_pair : Successors)
		{
			successor = successor_pair.second;
			// to prevent an infinite loop mark as visited
			if ( successor->hasBeenVisited() )
			{
				// if ( successor->hasSuccessors() )
				// {
				// 	tabCountLeft += 2;
				// }
				
				successor->setVisited(false);
				successor->prettyPrint(tabCountLeft+2*successorCount, tabCountRight, biOpCount);
				successorCount++;
			}
		}
	}

	// std::string insertSuccessor(){}

	std::string generateTabs(int count)
	{
		std::string tabs;
		for (int i = 0; i < count; ++i)
		{
			// if (i%2 != 0)
			// 	tabs += " \t";
			// else
			// 	tabs += " |\t";
			tabs += " \t";
		}
		return tabs;
	}
	
	void printSuccessorsNames()
	{
		for (auto successor_pair : Successors)
		{
      		llvm::outs() << successor_pair.second->getName() << " ";
		}
    	llvm::outs() << "\n";
	}

	void printPredecessorsNames()
	{
		for (auto predecessor_pair : Predecessors)
		{
      		llvm::outs() << predecessor_pair.second->getName() << " ";
		}
    	llvm::outs() << "\n";
	}


	bool DOTcreateNode(std::ostringstream &outSS, DAGNode* parentNode=nullptr)
	{
		// Declare node format:
		//	nodeName[label = "<label>"];
		auto nodeAddress = DOTnodeID();
		outSS << "\t" << nodeAddress << "[label = \"" << getName() << "\"];" << std::endl;

		// Link and add to graph
		//	"parentNode"->"node";
		if ( parentNode != nullptr)
		{
			outSS << "\t\"" << parentNode->DOTnodeID() << "\"->\"";
			outSS << nodeAddress << "\";" << std::endl;  
		}

		return true;
	}

	std::string DOTnodeID()
	{
		std::uintptr_t ptr_val = std::uintptr_t(&*llvmValue);
		std::stringstream ss;
		ss << std::dec << ptr_val;

		return ss.str();
	}

};


#endif /* DAG_NODE_H */ 
