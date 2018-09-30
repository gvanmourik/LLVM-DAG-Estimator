#ifndef DAG_BUILDER_H
#define DAG_BUILDER_H

// #include "DGBuilder.h"
#include "DAGNode.h"

class DAGBuilder 
{
private:
	// DGBuilder *DG_Builder; //dependence graph builder
	DAGVertexList Vertices;
	DAGValueList VertexValues;
	DAGValueList StoredValues;
	// DAGNameList keyByName;
	// int ID;
	// int NameCount;
	bool hasBeenInitialized;
	bool DAGIsLocked;


public:
	DAGBuilder() : hasBeenInitialized(false) {}
	~DAGBuilder() {}

	DAGVertexList retrieveDAG() 
	{
		assert( DAGIsLocked && "DAGBuilder has not been locked! Do so before retrieving the DAG.");
		return Vertices;
	}

	void lock() { DAGIsLocked = true; }
	// DGBuilder* getDGBuilder() 
	// { 
	// 	assert( DG_Builder != nullptr && "DGBuilder has not been set. Use createDG()");
	// 	return DG_Builder;
	// }

	void init()
	{
		Vertices.clear();
		DAGIsLocked = false;
		hasBeenInitialized = true;
	}

	/// DAG
	bool add(llvm::Value* value)
	{
		assert( hasBeenInitialized && "Builder has not been initialized! Do so with init()");
		
		if ( llvm::isa<llvm::Instruction>(value) )
		{
			/// Ignore the following
			auto inst = llvm::cast<llvm::Instruction>(value);
			if ( isInvalidOperator(inst) )
				return false;
			if ( isBranch(inst) )
				return false;
			// if ( isAlloca(inst) )
			// 	return false;
			if ( isReturn(inst) )
				return false;

			addInstruction(value);
			// add vertex for value (LHS)
			// add successor vertex with instruction
			// add successor to instruction with 1st operand value
			// add successor to instruction with 2nd operand value
		}

		
		// if ( isAlloca(value) )
		// 	return false;
		

		// addOperator(value);

		// llvm::outs() << "Value Address = " << &*value << "\n";

		return true;
	}

private:
	// void addOperator(llvm::Instruction *inst)
	// {
	// 	DAGNode *operatorNode;
	// 	// if instruction has name, then it WILL be unique
	// 	if ( inst->hasName() )
	// 	{
	// 		operatorNode = addVertex(inst, inst->getName());
	// 		addOperands(inst, operatorNode);
	// 	}
	// 	else
	// 	{
	// 		std::string operatorName = genValueName(inst);

	// 		if ( !isNamePresent(operatorName) )
	// 		{
	// 			operatorNode = addVertex(inst, operatorName);
	// 			addOperands(inst, operatorNode);
	// 		}
	// 	}
	// }

	// void addOperands(llvm::Instruction *inst, DAGNode *parentNode)
	// {
	// 	/// Add operands
	// 	// llvm::Value could be either an instruction or const value
	// 	llvm::Value *firstValue, *secondValue;
	// 	firstValue = inst->getOperand(0);
	// 	if ( hasTwoOperands(inst) )
	// 	{
	// 		secondValue = inst->getOperand(1);
	// 		addOperand(firstValue, parentNode); //first
	// 		addOperand(secondValue, parentNode); //second
	// 	}
	// 	else
	// 	{
	// 		addOperand(firstValue, parentNode);
	// 	}
	// }

	// void addOperand(llvm::Value *value, DAGNode* parentNode)
	// {
	// 	DAGNode *valueNode;

	// 	std::string valueName;
	// 	if ( value->hasName() )
	// 		valueName = value->getName();
	// 	else
	// 		valueName = genValueName(value);


	// 	if ( !isNamePresent(valueName) )
	// 		addVertex(value, valueName);

	// 	valueNode = DAGVertices[keyByName[valueName]];
	// 	addEdge(parentNode, valueNode);
	// }
	
	bool addInstruction(llvm::Value* value, DAGNode* parentNode=nullptr)
	{    
		auto inst = llvm::cast<llvm::Instruction>(value);
		if ( llvm::isa<llvm::AllocaInst>(inst) )
			return false;
		if ( llvm::isa<llvm::StoreInst>(inst) )
		{
			addStoreInst(value, parentNode);
			return true;
		}
		
		DAGNode *valNode, *instNode;

		if ( !isValuePresent(value) )
		{
			valNode = addVertex(value, VAL);
			instNode = addVertex(value, INST);
			instNode->setValueNode(valNode);
			addEdge(valNode, instNode);
		}
		else
		{
			auto key = VertexValues[value];
			instNode = Vertices[key];
			valNode = instNode->getValueNode();
		}

		// parentNode==nullptr indicates initial instruction
		if ( parentNode != nullptr )
		{
			addEdge(parentNode, valNode);
		}

		// /// If inst is an alloc, do NOT traverse the operands
		// if ( llvm::isa<llvm::AllocaInst>(inst) )
		// 	return false;

		/// Generate specific names for stores and loads
		std::string targetVarName;
		// if ( llvm::isa<llvm::StoreInst>(inst) )
		// {
		// 	targetVarName = inst->getOperand(1)->getName();
		// 	// StoreList[targetVarName] = instNode; ////
		// 	targetVarName = "store_" + targetVarName;


		// 	// test
		// 	// inst->getOperand(0)->dump();
		// 	// inst->getOperand(1)->dump();
		// 	// llvm::outs() << "Operand[0]->getValue = " << *(inst->getOperand(0)) << "\n";
		// 	// llvm::outs() << "Operand[1]->getValue = " << *(inst->getOperand(1)) << "\n";
		// 	// llvm::outs() << "Operand [1] = " << inst->getOperand(1) << "\n";
		// }
		if (llvm::isa<llvm::LoadInst>(inst) )
		{
			targetVarName = inst->getOperand(0)->getName();
			// if ( isStorePresent(targetVarName) ) ////
			// {
			// 	// get value node for the load
			// 	// move to the inst node for the load (as load/stores value nodes only have one successor)
			// 	auto loadInstNode = StoreList[targetVarName];

			// 	// replace the edge to the load_ inst with an edge to the store_ inst

			// }

			targetVarName = "load_" + targetVarName;
		}
		instNode->setConstName(targetVarName);
		valNode->setConstName(targetVarName);

		/// Recursively traverse each operand
		llvm::outs() << "Instruction: ";
		inst->dump();
		for (auto& operand : inst->operands())
		{
			llvm::outs() << "Operand: ";
			operand->dump();
			addOperand(operand, instNode);
		}
		return true;
	}

	bool addStoreInst(llvm::Value* value, DAGNode* parentNode)
	{
		auto inst = llvm::cast<llvm::Instruction>(value);
		// if ( llvm::isa<llvm::AllocaInst>(inst) )
		// 	return false;
		
		DAGNode *valNode, *instNode;

		// Is this check really necessary for store??
		if ( !isValuePresent(value) )
		{
			valNode = addVertex(value, VAL);
			instNode = addVertex(value, INST);
			instNode->setValueNode(valNode);
			addEdge(valNode, instNode);
		}
		else
		{
			auto key = VertexValues[value];
			instNode = Vertices[key];
			valNode = instNode->getValueNode();
		}

		// Name the store inst
		std::string targetVarName = inst->getOperand(1)->getName();
		targetVarName = "store_" + targetVarName;
		instNode->setConstName(targetVarName);
		valNode->setConstName(targetVarName);

		// Connect the newly created value node to the parent node
		if ( parentNode != nullptr )
			addEdge(parentNode, valNode);

		//the difference is in the addition of the operands
		auto saveToVar = inst->getOperand(1);
		auto storedValue = inst->getOperand(0);

		DAGNode* saveToVarNode = addVertex(saveToVar, VAL);
		addEdge(instNode, saveToVarNode);
		addEdge(saveToVarNode, addVertex(saveToVar, INST));

		addOperand(storedValue, saveToVarNode);
		

		return true;
	}

	////// IF parentNode == ????
	void addOperand(llvm::Value* operand, DAGNode* parentNode)
	{
		DAGNode* valNode;

		if ( llvm::isa<llvm::Instruction>(operand) )
		{
			addInstruction(operand, parentNode);
		}
		else
		{
			if ( !isValuePresent(operand) )
			{			
				std::string constValName;
				if ( llvm::isa<llvm::ConstantInt>(operand)
					&& !llvm::isa<llvm::AllocaInst>(operand) )
				{
					auto CI = llvm::cast<llvm::ConstantInt>(operand);
					if ( CI->getBitWidth() <= 64 )
					{
						int constInt = CI->getSExtValue();
						llvm::outs() << "constInt = " << constInt << "\n";
						constValName = std::to_string(constInt);
					}
				}
				if ( llvm::isa<llvm::ConstantFP>(operand) )
				{
					// add stuff...
				}

				valNode = addVertex(operand, VAL);
				valNode->setConstName(constValName);
			}
			else
			{
				auto key = VertexValues[operand];
				valNode = Vertices[key];
			}
			addEdge(parentNode, valNode);
		}
	}

	DAGNode* addVertex(llvm::Value* value, vertex_t type)
	{
		auto vertex = new DAGNode(value, type);
		Vertices[vertex] = vertex;
		VertexValues[value] = vertex;

		return vertex;
	}

	void addEdge(DAGNode* parentNode, DAGNode* successor)
	{
		parentNode->addSuccessor(successor);
	}

	// void replaceLoadEdge(DAGNode* loadNode, DAGNode* storeNode)
	// {
	// 	// Destroys all the the load_ instruction and consecutive nodes of that load_ inst
	// 	loadNode->removeSuccessors();
	// 	// Add edge to the store_ instruction
	// 	loadNode->addEdge(loadNode, storeNode);
	// }


	// std::string genValueName(llvm::Value *value)
	// {
	// 	std::string operatorName;
	//     if (llvm::isa<llvm::Instruction>(value) )
	// 	{
	//       	auto inst = llvm::cast<llvm::Instruction>(value);
	//       	if (llvm::isa<llvm::StoreInst>(inst) )
	// 		{
	// 			operatorName = inst->getOperand(1)->getName();
	// 			operatorName = "store_" + operatorName;
	// 		}
	//       	if (llvm::isa<llvm::LoadInst>(inst) )
	// 		{
	// 			operatorName = inst->getOperand(0)->getName();
	// 			operatorName = "load_" + operatorName;
	// 		}
	// 	}
	// 	return operatorName;
	// }

	// std::string genConstName()
	// {
	// 	NameCount++;
	// 	return "constVal_" + std::to_string(NameCount);
	// }
	
	bool isValuePresent(llvm::Value* key)
	{
		if( VertexValues.count(key) == 0 )
			return false;
		else
			return true;
	}

	// bool isStorePresent(std::string key)
	// {
	// 	if( StoreList.count(key) == 0 )
	// 		return false;
	// 	else
	// 		return true;
	// }
			
	// bool hasTwoOperands(llvm::Instruction *inst)
	// {
	// 	int count = 0;
	// 	for (auto iter=inst->op_begin(); iter!=inst->op_end(); ++iter)
	// 		count++;

	// 	if (count == 2)
	// 		return true;
	// 	else
	// 		return false;
	// }

	bool isInvalidOperator(llvm::Instruction *inst)
	{
		const char* opName = inst->getOpcodeName(inst->op_begin()->get()->getValueID());
		if (strcmp(opName, "<Invalid operator>") == 0)
			return true;
		else
			return false;
	}

	bool isBranch(llvm::Instruction *inst)
	{
    if (inst->getOpcode() == llvm::Instruction::Br)
			return true;
		else
			return false;
	}

	bool isAlloca(llvm::Instruction *inst)
	{
    if (inst->getOpcode() == llvm::Instruction::Alloca)
			return true;
		else
			return false;
	}

	bool isReturn(llvm::Instruction *inst)
	{
    if (inst->getOpcode() == llvm::Instruction::Ret)
			return true;
		else
			return false;
	}

public:
	// void collectAdjNodes()
	// {
	// 	assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
	// 	for (int i = 0; i < ID; ++i)
	// 	{
	// 		DAGVertices[i]->addAdjNodes( DAGVertices[i]->getLeft() );
	// 		DAGVertices[i]->addAdjNodes( DAGVertices[i]->getRight() );
	// 	}
	// }

	// void createDG()
	// {
	// 	assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
	// 	DG_Builder = new DGBuilder(DAGVertices, ID);
		
	// 	DG_Builder->createDG();
	// 	DG_Builder->createVDG();
	// 	// DG_Builder->createODG();
	// }

	// void fini()
	// {
	// 	collectAdjNodes();
	// 	createDG();
	// 	// createVDG();
	// }

	void print()
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
				llvm::outs() << "     Adjacent Nodes: ";
				vertex->printSuccessorsNames();
			}
			llvm::outs() << "-------------------------------------------------------------\n";
			i++;
		}
    	llvm::outs() << "\n";
	}

	// void printDependencyGraph()
	// {
	// 	assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
	// 	DG_Builder->printDG();
	// }
	

};


#endif /* DAG_BUILDER_H */
