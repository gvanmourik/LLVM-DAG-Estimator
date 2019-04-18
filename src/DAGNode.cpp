#include "DAGNode.h"

/// Returns a DAG node pointer if the successor exists.
DAGNode* DAGNode::getSuccessor(DAGNode* successor)
{
	if ( isSuccessorPresent(successor) )
		return Successors[successor];
	else
		return nullptr;
}

/// Adds a successor to the mapping.
bool DAGNode::addSuccessor(DAGNode* successor)
{
	if ( successor != nullptr || !isSuccessorPresent(successor) ) 
	{
		Successors[successor] = successor;
		return true;
	}
	return false;
}

/// Removes a successor from the mapping.
bool DAGNode::removeSuccessor(DAGNode* successor)
{
	if ( successor != nullptr || isSuccessorPresent(successor) ) 
	{
		Successors.erase(successor);
		return true;
	}
	return false;
}

/// Adds a predecessor to the mapping.
bool DAGNode::addPredecessor(DAGNode* predecessor)
{
	if ( predecessor != nullptr || !isPredecessorPresent(predecessor) ) 
	{
		Predecessors[predecessor] = predecessor;
		return true;
	}
	return false;
}

/// Get the node name.
std::string DAGNode::getName() 
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

/// Set the node name.
bool DAGNode::setName(std::string name)
{
	if ( llvmValue->hasName() )
	{
		return false;
	}
	llvmValue->setName(name);
	return true;
}

/// Set whether or not the node has been visited.
void DAGNode::setVisited(bool status)
{
	visited = status;
}

/// Set this node and all successors as unvisited.
void DAGNode::setAllUnvisited()
{
	visited = false;
	DAGNode* successor;
	for (auto successor_pair : Successors)
	{
		successor = successor_pair.second;
		successor->setVisited(false);
	}
}

/// Returns True if any of this nodes successors contains a store instruction.
bool DAGNode::hasStoredValue()
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

/// Returns True if this node has been visited, False otherwise.
bool DAGNode::hasBeenVisited()
{
	if (visited == true)
		return true;
	else
		return false;
}

/// Returns a pointer to the last node in the successors mapping that contains a store instruction. 
DAGNode* DAGNode::getStoredValueNode()
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

/// Returns True if the next node is a store instruction, False otherwise.
bool DAGNode::isStoreInstNext()
{
	if ( Successors.size() != 1 )
		return false;

	auto childNode = Successors.begin()->second;
	if ( llvm::isa<llvm::StoreInst>(childNode->getllvmValue()) )
	{
		return true;
	}
	return false;
}

/// Returns True if successors are present, False otherwise.
bool DAGNode::isSuccessorPresent(DAGNode* successor)
{
	if (Successors.count(successor) != 0)
		return true;
	else
		return false;
}

/// Returns True if predecessors are present, False otherwise.
bool DAGNode::isPredecessorPresent(DAGNode* predecessor)
{
	if (Predecessors.count(predecessor) != 0)
		return true;
	else
		return false;
}

/// Returns True if the node vertex_t is an instruction, False otherwise.
bool DAGNode::isaInst()
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

/// Prints the node, based on the vertex_t.
void DAGNode::print()
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

//needs work
void DAGNode::prettyPrint(int tabCountLeft, int tabCountRight, int biOpCount)
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

//helper for prettyPrint()
std::string DAGNode::generateTabs(int count)
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

/// Prints the names of the successor nodes.
void DAGNode::printSuccessorsNames()
{
	for (auto successor_pair : Successors)
	{
  		llvm::outs() << successor_pair.second->getName() << " ";
	}
	llvm::outs() << "\n";
}

/// Prints the names of the predecessor nodes.
void DAGNode::printPredecessorsNames()
{
	for (auto predecessor_pair : Predecessors)
	{
  		llvm::outs() << predecessor_pair.second->getName() << " ";
	}
	llvm::outs() << "\n";
}

/**
 * @brief      Create a node for the DOT file and link to the parent node.
 *
 * @param      outSS       The output string stream
 * @param      parentNode  The DAG parent node
 *
 * @return     Returns True if the node was successfully added to the DOT file.
 */
bool DAGNode::DOTcreateNode(std::ostringstream &outSS, DAGNode* parentNode)
{
	// Declare node format:
	//	nodeName[label = "<label>"];
	auto nodeAddress = DOTnodeID();
	outSS << "\t\t" << nodeAddress << "[label = \"" << getName() << "\"];" << std::endl;

	// Link and add to graph
	//	"parentNode"->"node";
	if ( parentNode != nullptr)
	{
		outSS << "\t\t\"" << parentNode->DOTnodeID() << "\"->\"";
		outSS << nodeAddress << "\";" << std::endl;  
	}

	return true;
}

/**
 * @brief      Generates a unique node id for the DOT file, based on the
 * 				address of the llvm value pointer.
 *
 * @return     A string with the decimal equivalent of the id.
 */
std::string DAGNode::DOTnodeID()
{
	std::uintptr_t ptr_val = std::uintptr_t(&*llvmValue);
	std::stringstream ss;
	ss << std::dec << ptr_val;

	return ss.str();
}

