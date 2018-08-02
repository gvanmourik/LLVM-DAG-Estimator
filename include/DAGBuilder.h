#ifndef DAG_BUILDER_H
#define DAG_BUILDER_H

#include "VDGBuilder.h"
#include "DAGNode.h"
#include "DepNode.h"

typedef std::unordered_map<std::string, llvm::Instruction*> DAGInstTracker;

class DAGBuilder 
{
private:
	VDGBuilder *vdgBuilder;
	DAGVertexList DAGVertices;
	DAGNameList keyByName;
	DAGInstTracker instByName;
	int ID;
	int NameCount;
	int WriteCount;
	bool hasBeenInitialized;
	bool DAGIsLocked;


public:
	DAGBuilder() : vdgBuilder(nullptr), ID(0), NameCount(0), WriteCount(0), hasBeenInitialized(false) {}
	~DAGBuilder() {}

	void lock() { DAGIsLocked = true; }
	VariableDependencyGraph getVDG() 
	{ 
		assert( vdgBuilder != nullptr && "VDGBuilder has not been set. Use createDependenceGraph()");
		return vdgBuilder->getVDG();
	}

	void init()
	{
		DAGVertices.clear();
		keyByName.clear();
		DAGIsLocked = false;
		hasBeenInitialized = true;
	}

	/// DAG
	bool add(llvm::Instruction *inst)
	{
		assert( hasBeenInitialized && "Builder has not been initialized! Do so with init()");
		/// Check if instruction is invalid operator (aka branch)
		if ( isInvalidOperator(inst) )
			return false;
		if ( isBranch(inst) )
			return false;
		if ( isAlloca(inst) )
			return false;

		// if ( isa<BinaryOperator>(inst) )
		// {
		// 	outs() << "Inst Name = " << inst->getName() << "\n";
		// 	if ( inst->getOperand(0)->hasName() )
		// 		/// INSTRUCTION
		// 		outs() << "PtrOpName = " << inst->getOperand(0)->getName() << "\n";

		// 	if ( isa<LoadInst>(inst->getOperand(0)) )
		// 	{
		// 		llvm::LoadInst *loadInst = cast<LoadInst>(inst->getOperand(0));
		// 		outs() << "     Test0 = " << loadInst->getPointerOperand()->getName() << "\n\n";
		// 	}
		// 	if ( isa<LoadInst>(inst->getOperand(1)) )
		// 	{
		// 		llvm::LoadInst *loadInst = cast<LoadInst>(inst->getOperand(1));
		// 		outs() << "     Test1 = " << loadInst->getPointerOperand()->getName() << "\n\n";
		// 	}
		// 	else
		// 	{
		// 		outs() << "constant value...\n";
		// 	}	
			
		// }
		// if ( isa<LoadInst>(inst) )
		// {
		// 	llvm::LoadInst *loadInst = cast<LoadInst>(inst);
		// 	outs() << "Test Load = " << loadInst->getPointerOperand()->getName() << "\n\n";
		// }

		DAGNode *instNode;

		/// Add operator
		if ( !inst->hasName() ) ///not doing anything....
		{
			std::string name = genLoadInstName(inst);
			if ( name != "<not a load>" )
			{
				if ( !isNamePresent(name) )
				{
					// outs() << "setting name to = " << name << "\n";

					if ( !isa<StoreInst>(inst) )
						inst->setName(name);
					instNode = addVertex(inst);
					// instByName[name] = inst;


					/// Add operands
					auto iter = inst->op_begin();
					llvm::Value *val = iter->get();
					if ( hasTwoOperands(inst) )
					{
						addOperand(val, inst, instNode); //first
						iter++;
						val = iter->get();
						addOperand(val, inst, instNode); //second
					}
					else
					{
						addOperand(val, inst, instNode);
					}
				}
				else
				{
					instNode = DAGVertices[keyByName[name]];
				}
			}
			return true;
		}
		
		instNode = addVertex(inst);

		/// Add operands
		auto iter = inst->op_begin();
		llvm::Value *val = iter->get();
		if ( hasTwoOperands(inst) )
		{
			addOperand(val, inst, instNode); //first
			iter++;
			val = iter->get();
			addOperand(val, inst, instNode); //second
		}
		else
		{
			addOperand(val, inst, instNode);
		}

		return true;
	}

private:
	void addEdge(DAGNode *parentNode, DAGNode *childNode)
	{
		if ( childNode != nullptr ) 
		{
			if ( parentNode->leftIsEmpty() )
				parentNode->setLeft(childNode);
			else
				parentNode->setRight(childNode);
		}
	}
	
	DAGNode* addVertex(llvm::Instruction *inst)
	{
		DAGVertices[ID] = new DAGNode(inst, ID);
		// std::string instName;

		// if ( inst->hasName() )
		// {
		// 	instName = inst->getName();
		// 	// instName = genInstName(inst);
		// 	// if ( instName == "<no name>" )
		// 	// 	instName = genName();
		// 	// if ( !isNamePresent(instName) )
		// 	// 	DAGVertices[ID]->setName(instName);
		// }
		// else
		// 	instName = genName();
		
		// DAGVertices[ID]->setName(instName);
		if ( inst->hasName() )
			DAGVertices[ID]->setName( inst->getName() );
		else if ( isa<StoreInst>(inst) )
		{
			std::string variableName = inst->getOperand(1)->getName();
			DAGVertices[ID]->setName("store_" + variableName);
		}
		else
			DAGVertices[ID]->setName( genName() );
		
		keyByName[inst->getName()] = ID;
		DAGNode *node = DAGVertices[ID];
		ID++;

		return node;
	}

	void addVertex(llvm::Value *value, llvm::Instruction *inst, std::string name)
	{
		DAGVertices[ID] = new DAGNode(inst, value, ID);
		DAGVertices[ID]->setName(name);

		keyByName[name] = ID;
		ID++;
	}

	void addOperand(llvm::Value *value, llvm::Instruction *inst, DAGNode* instNode)
	{
		DAGNode *operandNode;
		std::string name;
		if ( isa<Instruction>(value) )
			name = value->getName();
		else
			name = getNewName(value);

		if ( !isNamePresent(name) )
			addVertex(value, inst, name);
		else
			resetNameCount(value); //revert count used in genName()

		operandNode = DAGVertices[keyByName[name]];
		addEdge(instNode, operandNode);
	}

	std::string getNewName(llvm::Value *target)
	{
		if ( target->hasName() )
			return target->getName();
		else
			return genName();
	}

	std::string genLoadInstName(Instruction *inst)
	{
		// if ( inst->hasOneUse() )
		// 	return false;

		std::string instName;
		if ( isa<LoadInst>(inst) )
		{
			llvm::LoadInst *loadInst = cast<LoadInst>(inst);
			// outs() << "Test Load = " << loadInst->getPointerOperand()->getName() << "\n\n";
			instName = loadInst->getPointerOperand()->getName();
			return "load_" + instName;
		}
		if ( isa<StoreInst>(inst) )
			return "StoreInst";

		return "<not a load>";
	}

	std::string genName()
	{
		NameCount++;
		return "val_" + std::to_string(NameCount);
	}

	// std::string genStoreName()
	// {
	// 	WriteCount++;
	// 	return "Store_" + std::to_string(WriteCount);
	// }

	void resetNameCount(llvm::Value *target)
	{
		if ( !target->hasName() )
			NameCount--;
	}
	
	bool isNamePresent(std::string name)
	{
		if( keyByName.count(name) == 0 )
			return false;
		else
			return true;
	}
			
	bool hasTwoOperands(llvm::Instruction *inst)
	{
		int count = 0;
		for (auto iter=inst->op_begin(); iter!=inst->op_end(); ++iter)
			count++;

		if (count == 2)
			return true;
		else
			return false;
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
		if (inst->getOpcode() == Instruction::Br)
			return true;
		else
			return false;
	}

	bool isAlloca(llvm::Instruction *inst)
	{
		if (inst->getOpcode() == Instruction::Alloca)
			return true;
		else
			return false;
	}

public:
	void collectAdjNodes()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		for (int i = 0; i < ID; ++i)
		{
			DAGVertices[i]->addAdjNodes( DAGVertices[i]->getLeft() );
			DAGVertices[i]->addAdjNodes( DAGVertices[i]->getRight() );
		}
	}

	void createVDG()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		vdgBuilder = new VDGBuilder(DAGVertices, ID);
		vdgBuilder->createVDG();
	}

	void fini()
	{
		collectAdjNodes();
		createVDG();
	}

	void print()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		outs() << "\nDAG Nodes:\n";
		for (int i = 0; i < ID; ++i)
		{
			outs() << "-------------------------------------------------------------\n";
			outs() << "Node" << i << ":\n";
			DAGVertices[i]->print();
			outs() << "       Adjacent Nodes: ";
			DAGVertices[i]->printAdjNodeIDs();
			outs() << "-------------------------------------------------------------\n";
		}
		outs() << "\n";
	}

	void printDependencyGraph()
	{
		assert( DAGIsLocked && "DAG has not been locked! Do so with lock()");
		vdgBuilder->printVDG();
	}
	

};


#endif /* DAG_BUILDER_H */