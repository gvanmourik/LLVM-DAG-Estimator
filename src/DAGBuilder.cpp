#include "DAGBuilder.h"

/**
 * @brief      Adds a llvm value to the DAG.
 *
 * @param      value  The generalized llvm value
 *
 * @return     Returns whether or not the add was successful.
 */
bool DAGBuilder::add(llvm::Value* value)
{
	assert( hasBeenInitialized && "Builder has not been initialized! Do so with init()");
	
	if ( llvm::isa<llvm::Instruction>(value) )
	{
		/// Ignore the following types of instructions
		auto inst = llvm::cast<llvm::Instruction>(value);
		if ( isInvalidOperator(inst) )
			return false;
		if ( isBranch(inst) )
			return false;
		if ( isReturn(inst) )
			return false;

		addInstruction(value);
	}
	return true;
}

/**
 * @brief      Adds an llvm instruction to the DAG.
 *
 * @param      value       The llvm value
 * @param      parentNode  The DAG parent node
 *
 * @return     Returns whether or not the add was successful.
 */
bool DAGBuilder::addInstruction(llvm::Value* value, DAGNode* parentNode)
{    
	auto inst = llvm::cast<llvm::Instruction>(value);

	// ignore the following types of instructions
	if ( llvm::isa<llvm::AllocaInst>(inst) )
		return false;
	if ( llvm::isa<llvm::CmpInst>(inst) )
		return false;

	DAGNode* valNode=nullptr;
	DAGNode* instNode=nullptr;	


	// store inst
	if ( llvm::isa<llvm::StoreInst>(inst) ) 
	{
		addStoreInst(valNode, instNode, inst);
	}
	// load inst (load values only have one operand, the load value)
	else if ( llvm::isa<llvm::LoadInst>(inst) )
	{
		addLoadInst(valNode, instNode, inst);
	}
	// other inst
	else
	{
		checkAndSet(valNode, instNode, inst);

		// recursively add operands
		addOperand(inst->getOperand(0), instNode); // left operand
		addOperand(inst->getOperand(1), instNode); // right operand
	}

	// add an edge between parent node and the value node, which at 
	//	this point is linked to other collected nodes
	if ( parentNode != nullptr )
	{
		addEdge(parentNode, valNode);
	}
	
	return true;
}

/**
 * @brief      Adds a llvm store instruction to the DAG.
 *
 * @param      valNode    The DAG value node
 * @param      instNode   The DAG instance node
 * @param      storeInst  The llvm store instruction
 */
void DAGBuilder::addStoreInst(DAGNode* &valNode, DAGNode* &instNode, llvm::Instruction *storeInst)
{
	//instruction example: [store %type %storeVal, %pointerType %targetVal]

	llvm::Value *storeVal, *targetVal;
	storeVal = storeInst->getOperand(0);
	targetVal = storeInst->getOperand(1);
	
	// add target variable value node
	if ( isValuePresent(targetVal) )
	{
		auto key = VertexByValue[targetVal];
		valNode = Vertices[key];
	}
	else
	{
		valNode = addVertex(targetVal, VAL);
	}

	// set the name for the instNode
	std::string storeValName, targetValName, storeInstName;
	if ( storeVal->hasName() )
	{
		storeValName = storeVal->getName();				
	}
	else
	{
		storeValName = getConstName(storeVal);
	}
	if ( targetVal->hasName() )
	{
		targetValName = targetVal->getName();				
	}
	storeInstName = "store(" + storeValName + " to " + targetValName + ")";


	// store instructions will always have a unique ID
	instNode = addVertex(storeInst, INST);
	instNode->setConstName(storeInstName);
	instNode->setVarWidth(1);
	instNode->setVarDepth(1);

	// add edges
	addEdge(valNode, instNode);

	// recursively add the store value
	addOperand(storeVal, instNode);
}

/**
 * @brief      Adds a llvm load instruction to the DAG.
 *
 * @param      valNode    The DAG value node
 * @param      instNode   The DAG instance node
 * @param      loadInst  The llvm load instruction
 */
void DAGBuilder::addLoadInst(DAGNode* &valNode, DAGNode* &instNode, llvm::Instruction *loadInst)
{
	//instruction example: [%val = load i32, i32* %loadVal]

	llvm::Value *loadVal;
	DAGNode *loadValNode = nullptr;
	loadVal = loadInst->getOperand(0);

	checkAndSet(valNode, instNode, loadInst);

	// The load value node should always exists
	auto key = VertexByValue[loadVal];
	loadValNode = Vertices[key];

	// get name
	std::string loadValName = loadVal->getName();
	loadValName = "load_" + loadValName;

	// set names 
	valNode->setConstName("val_" + loadValName);
	instNode->setConstName("inst_" + loadValName);

	//Add edges
	addEdge(instNode, loadValNode);
}

/**
 * @brief      Check if the llvm value is present and the set the value and 
 * 				inst nodes accordingly
 *
 * @param      valNode   The DAG value node
 * @param      instNode  The DAG instruction node
 * @param      value     The llvm value
 */
void DAGBuilder::checkAndSet(DAGNode* &valNode, DAGNode* &instNode, llvm::Value *value)
{
	if ( isValuePresent(value) )
	{
		auto key = VertexByValue[value];
		instNode = Vertices[key];
		valNode = instNode->getValueNode();
	}
	else
	{
		// create a value and inst nodes for the new llvm::value
		valNode = addVertex(value, VAL);
		instNode = addVertex(value, INST);
		instNode->setValueNode(valNode);

		// add an edge valNode->instNode
		addEdge(valNode, instNode);
	}
}

/**
 * @brief      Adds a llvm instruction operand. Recursively calls addInstruction()
 * 				if the operand is an instruction. Otherwise, it adds the value as
 * 				a leaf in the DAG. 
 *
 * @param      operand     The llvm operand
 * @param      parentNode  The DAG parent node
 */
void DAGBuilder::addOperand(llvm::Value* operand, DAGNode* parentNode)
{
	DAGNode* valNode;

	if ( llvm::isa<llvm::Instruction>(operand) )
	{
		addInstruction(operand, parentNode);
	}
	else
	{
		if ( isValuePresent(operand) )
		{	
			auto key = VertexByValue[operand];
			valNode = Vertices[key];
		}
		else
		{
			std::string constValName;
			constValName = getConstName(operand);
			valNode = addVertex(operand, VAL);
			valNode->setConstName(constValName);
		}
		addEdge(parentNode, valNode);
	}
}

/**
 * @brief      Gets the name for a constant llvm value. 
 *
 * @param      constVal  The constant value
 *
 * @return     The constant name for the corresponding value node.
 */
std::string DAGBuilder::getConstName(llvm::Value* constVal)
{
	std::string constValName;
	if ( llvm::isa<llvm::ConstantInt>(constVal) )
	{
		auto CI = llvm::cast<llvm::ConstantInt>(constVal);
		if ( CI->getBitWidth() <= 64 )
		{
			int constInt = CI->getSExtValue();
			constValName = std::to_string(constInt);
		}
	}
	if ( llvm::isa<llvm::ConstantFP>(constVal) )
	{
		// add stuff...
	}

	return constValName;
}

/**
 * @brief      Adds a vertex to the DAG.
 *
 * @param      value  The llvm value
 * @param[in]  type   The vertex type (defined in DAGNode.h)
 *
 * @return     Returns a pointer to the added DAG vertex.
 */
DAGNode* DAGBuilder::addVertex(llvm::Value* value, vertex_t type)
{
	auto vertex = new DAGNode(value, type);
	Vertices[vertex] = vertex;
	VertexByValue[value] = vertex;

	return vertex;
}

/**
 * @brief      Adds a double edge between two DAG nodes.
 *
 * @param      currentNode  The current node
 * @param      nextNode     The next node
 */
void DAGBuilder::addEdge(DAGNode* currentNode, DAGNode* nextNode)
{
	currentNode->addSuccessor(nextNode);
	nextNode->addPredecessor(currentNode);
}

/**
 * @brief      Determines if a llvm value is present in the DAG vertex map.
 *
 * @param      key   The key
 *
 * @return     True if value present, False otherwise.
 */
bool DAGBuilder::isValuePresent(llvm::Value* key)
{
	if( VertexByValue.count(key) == 0 )
		return false;
	else
		return true;
}

/**
 * @brief      Determines if the llvm instruction is an invalid operator.
 *
 * @param      inst  The llvm instruction
 *
 * @return     True if invalid operator, False otherwise.
 */
bool DAGBuilder::isInvalidOperator(llvm::Instruction *inst)
{
	const char* opName = inst->getOpcodeName(inst->op_begin()->get()->getValueID());
	if (strcmp(opName, "<Invalid operator>") == 0)
		return true;
	else
		return false;
}

/**
 * @brief      Determines if the llvm instruction forms a branch.
 *
 * @param      inst  The llvm instruction
 *
 * @return     True if it is a branch, False otherwise.
 */
bool DAGBuilder::isBranch(llvm::Instruction *inst)
{
	if (inst->getOpcode() == llvm::Instruction::Br)
		return true;
	else
		return false;
}

/**
 * @brief      Determines if is the llvm instruction is an allocation.
 *
 * @param      inst  The llvm instruction
 *
 * @return     True if allocation, False otherwise.
 */
bool DAGBuilder::isAlloca(llvm::Instruction *inst)
{
	if (inst->getOpcode() == llvm::Instruction::Alloca)
		return true;
	else
		return false;
}

/**
 * @brief      Determines if the llvm instruction is a return instance.
 *
 * @param      inst  The llvm instruction
 *
 * @return     True if it is a return instance, False otherwise.
 */
bool DAGBuilder::isReturn(llvm::Instruction *inst)
{
	if (inst->getOpcode() == llvm::Instruction::Ret)
		return true;
	else
		return false;
}

///
/// Traversal: 
///  - The following functions do work, but in a limited manner
///  - findHeight() currently only finds the variable dependence height
///  - A section needs to be added in the findHeightWrapper() to track 
///  	the op height
///  - Functions also need to be added for the width
///
bool DAGBuilder::findSourceNodes(std::vector<DAGNode*> &sourceNodes)
{
	if ( Vertices.empty() )
		return false;

	sourceNodes.clear();
	for (auto vertex_pair : Vertices)
	{
		if ( !vertex_pair.second->hasPredecessors() )
			sourceNodes.push_back(vertex_pair.second);
	}
	return true;
}

int DAGBuilder::findHeight()
{
	std::vector<DAGNode*> sourceNodes;
	if ( !findSourceNodes(sourceNodes) )
	{
		llvm::outs() << " ERROR: No source nodes were found!\n";
		return false;
	}
	if ( sourceNodes.empty() )
		return 0;

	//vector to record the depths of each source/root node
	std::vector<int> sourceDepths(sourceNodes.size());
	int index = 0;

	for ( auto sourceNode : sourceNodes )
	{
		//recursive helper funciton that is fed each source/root node
		sourceNode->setAllUnvisited();
		sourceDepths[index] = findHeightWrapper(sourceNode);
		index++;
	}

	//find maxDepth
	int maxDepth = 1;
	for ( auto depth : sourceDepths )
	{
		if ( depth > maxDepth )
			maxDepth = depth;
	}

	return maxDepth;
}

int DAGBuilder::findHeightWrapper(DAGNode *node)
{
	node->setVisited(true);
	if ( !node->hasSuccessors() )
		return 1;

	auto Successors = node->getSuccessors();
	std::vector<int> depths(Successors.size());
	int index = 0;

	for ( auto successor_pair : Successors )
	{
		auto successor = successor_pair.second;
		if ( !successor->hasBeenVisited() )
		{

			//find total varDepth
			if ( successor->isStoreInstNext() )
			{
				depths[index] = findHeightWrapper(successor) + 1;
			}
			else
			{
				depths[index] = findHeightWrapper(successor);
			}
			index++;
		}
	}

	return *std::max_element( depths.begin(), depths.end() );
}

/**
 * @brief      Print the DAG nodes.
 */
void DAGBuilder::print()
{
	assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
	llvm::outs() << "\nDAG Nodes:\n";
	DAGNode *vertex;
	int i = 0;
	for (auto vertex_pair : Vertices)
	{
		vertex = vertex_pair.second;
		llvm::outs() << "-------------------------------------------------------------\n";
		llvm::outs() << "Node" << i << ":\n";
		vertex->print();
		if ( vertex->hasSuccessors() )
		{
			llvm::outs() << "           Next Nodes: ";
			vertex->printSuccessorsNames();
			llvm::outs() << "       Previous Nodes: ";
			vertex->printPredecessorsNames();
		}

		// llvm::outs() << "\nPretty Print...\n";
		// vertex->prettyPrint();
		// 
		llvm::outs() << "-------------------------------------------------------------\n";
		i++;
	}
	llvm::outs() << "\n";
}

/**
 * @brief      Generate a DOT file of the DAG.
 *
 * @param[in]  functionName  The llvm function name
 *
 * @return     Returns True on the successful DOT file generation, and False
 * 				if there is an error.
 */
bool DAGBuilder::DOTGenerateFile(std::string functionName)
{
	std::vector<DAGNode*> sourceNodes;
	if ( !findSourceNodes(sourceNodes) )
	{
		llvm::outs() << " ERROR: No source nodes were found!\n";
		return false;
	}

	// stream to collect file output
	std::ostringstream outSS;

	// init graph and node specs
	outSS << "digraph g{" << std::endl;
	outSS << "\tnode [shape = record,height = .1];" << std::endl << std::endl;
	outSS << "\tlabelloc = \"t\";" << std::endl;
	outSS << "\tlabel = \"" << functionName << "_DAG" << "\";" << std::endl;

	// generate node markup for each source/root node
	for (auto root : sourceNodes)
	{
		outSS << std::endl;
		outSS << "\tsubgraph " << root->getName() << " {" << std::endl;
		root->DOTcreateNode(outSS);
		DOTAddSuccessor(outSS, root);
		outSS << "\t}" << std::endl;
	}
	outSS << "}" << std::endl;

	// Create and add to the .dot file
	std::string filePath = "../../dotFiles/" + functionName + "_Graph.dot";
	std::ofstream dotFile(filePath);
	if ( dotFile.is_open() )
	{
		dotFile << outSS.str();
		dotFile.close();
	}
	else
	{
		llvm::outs() << " ERROR: DOT file did not open!\n";
		return false;
	}

	llvm::outs() << " -- DOT file generated at: " << filePath << "\n";
	return true;
}

/**
 * @brief      Recursive helper function for DOTGenerateFile().
 *
 * @param      outSS       The output string stream
 * @param      parentNode  The DAG parent node
 *
 * @return     Returns True if the DAG successor node was added to the DOT, 
 * 				and False otherwise.
 */
bool DAGBuilder::DOTAddSuccessor(std::ostringstream &outSS, DAGNode* parentNode)
{
	if ( !parentNode->hasSuccessors() )
		return false;

	auto Successors = parentNode->getSuccessors();
	DAGNode *successor;
	for (auto successor_pair : Successors)
	{
		successor = successor_pair.second;
		// to prevent an infinite loop mark as visited
		if ( !successor->hasBeenVisited() )
		{
			successor->setVisited(true);
			successor->DOTcreateNode(outSS, parentNode);
			DOTAddSuccessor(outSS, successor);
		}
	}
	parentNode->setAllUnvisited(); //reset visited status

	return true;
}

