#ifndef DAG_BUILDER_H
#define DAG_BUILDER_H

// #include "DGBuilder.h"
#include "DAGNode.h"

class DAGBuilder 
{
private:
	// DGBuilder *DG_Builder; //dependence graph builder
	DAGVertexList Vertices;
	DAGValueList VertexByValue;
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

		DAGNode *valNode, *instNode;		

		///// NEW
		/// Store
		if ( llvm::isa<llvm::StoreInst>(inst) )
		{
			llvm::Value *storeVal, *targetVal;
			storeVal = inst->getOperand(0);
			targetVal = inst->getOperand(1);
			
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

			
			// determine type
			vertex_t storeType;
			if ( llvm::isa<llvm::Instruction>(storeVal) )
			{
				storeType = INST;
			}
			if ( llvm::isa<llvm::Constant>(storeVal) )
			{
				storeType = CONST;
			}
			
			// set name
			std::string storeValName;
			if ( storeVal->hasName() )
			{
				storeValName = storeVal->getName();
				
			}
			else
			{
				storeValName = getConstName(storeVal);
			}
			storeValName = "store_" + storeValName;

			// store instructions will always have a unique ID
			instNode = addVertex(storeVal, storeType);
			instNode->setConstName(storeValName);

			// add edges
			if ( parentNode != nullptr )
			{
				addEdge(parentNode, valNode);
			}
			addEdge(valNode, instNode);

			// recursively add the store value
			if ( storeType == INST )
			{
				addOperand(storeVal, instNode);
			}
		}

		/// Load (load values only have one operand, the load value)
		else if ( llvm::isa<llvm::LoadInst>(inst) )
		{
			llvm::Value *loadVal, *targetVal;
			targetVal = inst;
			loadVal = inst->getOperand(0);

			// create load_ val and inst nodes
			// as with store, all load will be unique
			valNode = addVertex(targetVal, VAL);
			instNode = addVertex(targetVal, INST);

			// get name
			std::string loadValName = loadVal->getName();
			loadValName = "load_" + loadValName;

			// set names
			valNode->setConstName(loadValName);
			instNode->setConstName(loadValName);

			// add edge
			addEdge(valNode, instNode);

			// recursively add the load value
			addOperand(loadVal, instNode);
		}

		/// Other
		else
		{
			// each operation has a unique pointer
			// no need to check if value is present
			if ( isValuePresent(inst) )
			{
				auto key = VertexByValue[inst];
				instNode = Vertices[key];
				valNode = instNode->getValueNode();
			}
			else
			{
				valNode = addVertex(inst, VAL);
				instNode = addVertex(inst, INST);
				instNode->setValueNode(valNode);
				addEdge(valNode, instNode);
			}
			

			inst->dump();

			// recursively add operands
			addOperand(inst->getOperand(0), instNode); // left operand
			addOperand(inst->getOperand(1), instNode); // right operand


		}



		
		// DAGNode *valNode, *instNode;

		// if ( !isValuePresent(value) )
		// {
		// 	valNode = addVertex(value, VAL);
		// 	instNode = addVertex(value, INST);
		// 	instNode->setValueNode(valNode);
		// 	addEdge(valNode, instNode);
		// }
		// else
		// {
		// 	auto key = VertexByValue[value];
		// 	instNode = Vertices[key];
		// 	valNode = instNode->getValueNode();
		// }

		// // parentNode==nullptr indicates initial instruction
		// if ( parentNode != nullptr )
		// {
		// 	addEdge(parentNode, valNode);
		// }


		// std::string targetVarName;
		// if (llvm::isa<llvm::LoadInst>(inst) )
		// {
		// 	targetVarName = inst->getOperand(0)->getName();
		// 	targetVarName = "load_" + targetVarName;
		// }
		// instNode->setConstName(targetVarName);
		// valNode->setConstName(targetVarName);

		// /// Recursively traverse each operand
		// llvm::outs() << "Instruction: ";
		// inst->dump();
		// // if ( !llvm::isa<llvm::AllocaInst>(inst) )
		// // {
		// // 	for (auto& operand : inst->operands())
		// // 	{
		// // 		llvm::outs() << "Operand: ";
		// // 		operand->dump();
		// // 		addOperand(operand, instNode);
		// // 	}
		// // }
		
		return true;
	}

	////// IF parentNode == ????
	void addOperand(llvm::Value* operand, DAGNode* parentNode)
	{
		DAGNode* valNode;

		if ( llvm::isa<llvm::Instruction>(operand) )
		{
			printf("adding Instruction...\n");
			addInstruction(operand, parentNode);
		}
		else
		{
			operand->dump();
			int STOP;
			std::cin>>STOP;

			if ( !isValuePresent(operand) )
			{		
				printf("NOT present!!!\n");	
				std::string constValName;
				// if ( llvm::isa<llvm::Constant>(operand) )
				// {
					constValName = getConstName(operand);
				// }
				valNode = addVertex(operand, CONST);
				valNode->setConstName(constValName);
			}
			else
			{
				printf("I'm here!!!\n");
				auto key = VertexByValue[operand];
				valNode = Vertices[key];
			}
			addEdge(parentNode, valNode);
		}
	}

	std::string getConstName(llvm::Value* constVal)
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

	DAGNode* addVertex(llvm::Value* value, vertex_t type)
	{
		auto vertex = new DAGNode(value, type);
		Vertices[vertex] = vertex;
		VertexByValue[value] = vertex;

		return vertex;
	}

	void addEdge(DAGNode* parentNode, DAGNode* successor)
	{
		parentNode->addSuccessor(successor);
	}


	// bool addStoreInst(llvm::Value* value)
	// {
	// 	auto inst = llvm::cast<llvm::Instruction>(value);
	// 	// if ( llvm::isa<llvm::AllocaInst>(inst) )
	// 	// 	return false;
		
	// 	DAGNode *instNode, *saveToNode;

	// 	// Is this check really necessary for store??
	// 	if ( !isValuePresent(value) )
	// 	{
	// 		// valNode = addVertex(value, VAL);
	// 		// value node not needed, as the the value becomes the stored value
	// 		instNode = addVertex(value, INST); 
	// 		// instNode->setValueNode(valNode);
	// 		// addEdge(valNode, instNode);
	// 	}
	// 	else
	// 	{
	// 		auto key = VertexByValue[value];
	// 		instNode = Vertices[key];
	// 	}

	// 	// Name the store inst
	// 	std::string targetVarName = inst->getOperand(1)->getName();
	// 	targetVarName = "store_" + targetVarName;
	// 	instNode->setConstName(targetVarName);

	// 	// Connect the newly created value node to the parent node


	// 	// 1) Create or retrieve the node for the second operand (the pointer)
	// 	// 2) Add and edge between that node and the inst node (the store inst)
	// 	// 3) Create or retrieve the node for the first operand (the stored value)
		
	// 	// Pointer to the location where the value will be saved
	// 	auto saveToValue = inst->getOperand(1);
	// 	if ( !isValuePresent(saveToValue) )
	// 	{
	// 		saveToNode = addVertex(saveToValue, VAL);
	// 	}
	// 	else
	// 	{
	// 		auto key = VertexByValue[saveToValue];
	// 		saveToNode = Vertices[key];
	// 		saveToNode = saveToNode->getValueNode(); //retrieve value not inst node
	// 	}

	// 	// Add edges between the parent and the three successor nodes
	// 	// parent->saveToNode
	// 	// saveToNode->instNode
	// 	// saveToNode->print();
	// 	// printf("saveToNode Name = %s\n", saveToNode->getName().c_str());
	// 	if ( saveToNode->hasStoredValue() )
	// 	{
	// 		auto oldStoreNode = saveToNode->getStoredValueNode();
	// 		// auto newStoreNode = instNode->getStoredValueNode();
	// 		// if ID of old store == new store, do nothing
	// 		// else, remove old and add new
	// 		auto oldID = oldStoreNode->getContent();
	// 		auto newID = value;
	// 		if ( newID != oldID )
	// 		{
	// 			printf("------>IDs are NOT equal\n");
	// 			newID->dump();
	// 			oldID->dump();
	// 			saveToNode->removeSuccessor(oldStoreNode);
	// 			addEdge(saveToNode, instNode);
	// 			auto newValue = inst->getOperand(0);
	// 			addOperand(newValue, instNode);
	// 		}
	// 		else
	// 		{
	// 			printf("------>IDs ARE equal\n");
	// 		}
	// 	}
	// 	else
	// 	{
	// 		printf("------>Variable does NOT have stored values\n");
	// 		addEdge(saveToNode, instNode);
	// 		auto newValue = inst->getOperand(0);
	// 		addOperand(newValue, instNode);
	// 	}
		



	// 	// //the difference is in the addition of the operands
	// 	// auto saveToVar = inst->getOperand(1);
	// 	// auto storedValue = inst->getOperand(0);

	// 	// DAGNode* saveToVarNode = addVertex(saveToVar, VAL);
	// 	// addEdge(instNode, saveToVarNode);
	// 	// addEdge(saveToVarNode, addVertex(saveToVar, INST));

	// 	// addOperand(storedValue, saveToVarNode);
		

	// 	return true;
	// }

	

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
		if( VertexByValue.count(key) == 0 )
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
