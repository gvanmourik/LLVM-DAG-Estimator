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
			if ( isReturn(inst) )
				return false;

			addInstruction(value);
		}

		return true;
	}

private:
	
	bool addInstruction(llvm::Value* value, DAGNode* parentNode=nullptr)
	{    
		auto inst = llvm::cast<llvm::Instruction>(value);

		// remove unneeded instructions
		if ( llvm::isa<llvm::AllocaInst>(inst) )
			return false;
		if ( llvm::isa<llvm::CmpInst>(inst) )
			return false;

		DAGNode *valNode=nullptr, *instNode=nullptr;		


		/// Store
		if ( llvm::isa<llvm::StoreInst>(inst) ) 
		{
			addStoreInst(valNode, instNode, inst);
		}

		/// Load (load values only have one operand, the load value)
		else if ( llvm::isa<llvm::LoadInst>(inst) )
		{
			addLoadInst(valNode, instNode, inst);
		}

		/// Other
		else
		{
			checkAndSet(valNode, instNode, inst);

			// recursively add operands
			inst->dump();
			addOperand(inst->getOperand(0), instNode); // left operand
			addOperand(inst->getOperand(1), instNode); // right operand
		}

		// add children to parent
		if ( parentNode != nullptr )
		{
			addEdge(parentNode, valNode);
		}
		
		return true;
	}

	void addStoreInst(DAGNode* &valNode, DAGNode* &instNode, llvm::Instruction *storeInst)
	{
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
		instNode = addVertex(storeInst, INST);
		instNode->setConstName(storeValName);

		// add edges
		addEdge(valNode, instNode);

		// recursively add the store value
		addOperand(storeVal, instNode);
	}

	void addLoadInst(DAGNode* &valNode, DAGNode* &instNode, llvm::Instruction *loadInst)
	{
		llvm::Value *loadVal;
		loadVal = loadInst->getOperand(0);

		checkAndSet(valNode, instNode, loadInst);

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

	void checkAndSet(DAGNode* &valNode, DAGNode* &instNode, llvm::Value *value)
	{
		if ( isValuePresent(value) )
		{
			auto key = VertexByValue[value];
			instNode = Vertices[key];
			valNode = instNode->getValueNode();
		}
		else
		{
			valNode = addVertex(value, VAL);
			instNode = addVertex(value, INST);
			instNode->setValueNode(valNode);
			addEdge(valNode, instNode);
		}
	}

	void addOperand(llvm::Value* operand, DAGNode* parentNode)
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
	
	bool isValuePresent(llvm::Value* key)
	{
		if( VertexByValue.count(key) == 0 )
			return false;
		else
			return true;
	}

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
