#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/IR/Constants.h"
#include <map>
#include <iostream>
using namespace llvm;

bool checkConstant(Value * V) {
	if (isa<Constant>(V)) {
		return true;
	}
	else {
		return false;
	}
}

void createAndReplace(CallInst * callee) {
	LLVMContext & context = callee->getCalledFunction()->getContext();
	IntegerType * i32_type = IntegerType::getInt32Ty(context);

	for (int i = 0; i < callee->getNumArgOperands(); i++) {
		Value * arg = callee->getArgOperand(i);

		if (checkConstant(arg)) {
			Constant * argConstant = cast<Constant>(arg);
			uint64_t argInt = argConstant->getUniqueInteger().getZExtValue();
			ConstantInt * c_int = ConstantInt::get(i32_type, argInt);
			arg->replaceAllUsesWith(c_int);
		}
	}
}

int countInstructions(Function * F) {
	errs() << "Entering count instructions\n";
	int count = 0;
	errs() << "Checking for seg fault\n";
	errs() << F->getName();
	errs() << "\n";

	for (Function::iterator it = F->begin(); it != F->end(); it++) {
		errs() << "Entering count function iteration\n";
		BasicBlock & B = *it;
        for (BasicBlock::iterator b_it = B.begin(); b_it != B.end(); b_it++) {
        	errs() << "Entering count block iteration\n";
        	count += 1;
        }
    }

    errs() << "Leaving count instructions\n";
    return count;
}

int startInlineCI(CallInst * callI) {
	errs() << "Entering startInlineCI\n";
	Function & F = *callI->getCalledFunction();
	int numCalls = 0;
	std::map <std::string, int> numInstr;
	// Iterate over each function in the Module and call the replacement functions on it
	for (Function::iterator it = F.begin(); it != F.end(); it++) {
		errs() << "Entering function iteration\n";
		BasicBlock & B = *it;
        for (BasicBlock::iterator b_it = B.begin(); b_it != B.end(); b_it++) {
        	errs() << "Entering basicblock iteration\n";
        	Instruction & I =  *b_it;
        	errs() << "Got instruction\n";
		    if (CallInst * CI = dyn_cast<CallInst>(&I)) {
		    	errs() << "Found a call instr\n";
		    	Function * F = CI->getCalledFunction();
		    	if (F && !F->isDeclaration()) {
		    		errs() << "Is not a declaration\n";
		    		numInstr[CI->getCalledFunction()->getName()] = countInstructions(CI->getCalledFunction());
		    		errs() << numInstr[CI->getCalledFunction()->getName()];
			        errs() << "\n";

			    	errs() << "Back in startInline\n";
			    	
			    	/*
			        if (numInstr[CI->getCalledFunction()->getName()] < 10) {
			        	errs() << "beginning recursive call\n";
			        	startInlineCI(CI);
			        }
			        */

			        numCalls += 1;
			        errs() << numCalls;
		    	}
		    }
        }
     }

     if (numCalls == 0) {
     	// Call function to replace arguments with constant values
     	std::cout << "Reached leaf node";
     	createAndReplace(callI);
     }
     else {
     	for (Function::iterator it = F.begin(); it != F.end(); it++) {
     		BasicBlock & B = *it;
	        for (BasicBlock::iterator b_it = B.begin(); b_it != B.end(); b_it++) {
	        	Instruction & I = *b_it;
	          if (CallInst * CI = dyn_cast<CallInst>(&I)) {
	            // We know we've encountered a call instruction, so we
	            // need to determine if it's a call to the
	            // function pointed to by m_func or not.
	            
	            if (numInstr[CI->getCalledFunction()->getName()] < 10) {
		        	ValueToValueMapTy vmap;
		        	Function * F = CI->getCalledFunction();

					for (Function::iterator it = F->begin(); it != F->end(); it++) {
						BasicBlock & B = *it;
						for (BasicBlock::iterator b_it = B.begin(); b_it != B.end(); b_it++) {
							Instruction & inst = *b_it;
							Instruction * new_inst = inst.clone();
							B.getInstList().insert(&inst, new_inst);
							vmap[&inst] = new_inst;
							RemapInstruction(new_inst, vmap, RF_NoModuleLevelChanges);							
						}

					}
		        	errs() << "This function would be cloned\n";
		        	errs() << CI->getCalledFunction()->getName();
		        	errs() << "\n";
		        }
	          }
	        }
	     }
     }

     return 0;
}

int startInline(Function & F) {
	errs() << "Entering startInline\n";
	int numCalls = 0;
	std::map <std::string, int> numInstr;
	// Iterate over each function in the Module and call the replacement functions on it
	for (Function::iterator it = F.begin(); it != F.end(); it++) {
		errs() << "Entering function iteration\n";
		BasicBlock & B = *it;
        for (BasicBlock::iterator b_it = B.begin(); b_it != B.end(); b_it++) {
        	errs() << "Entering basicblock iteration\n";
        	Instruction & I =  *b_it;
        	errs() << "Got the instruction from the basicblock iterator\n";
		    if (CallInst * CI = dyn_cast<CallInst>(&I)) {
		    	errs() << "Found a call instr\n";
		    	Function * F = CI->getCalledFunction();
		    	errs() << "Got the call instruction Function\n";
		    	if (F && !F->isDeclaration()) {
		    		numInstr[CI->getCalledFunction()->getName()] = countInstructions(CI->getCalledFunction());
		    		errs() << numInstr[CI->getCalledFunction()->getName()];
			        errs() << "\n";

			    	errs() << "Back in startInline\n";
			    	
			    	/*
			        if (numInstr[CI->getCalledFunction()->getName()] < 10) {
			        	errs() << "beginning recursive call\n";
			        	startInline(*CI->getCalledFunction());
			        }
			        */

			        numCalls += 1;
			        errs() << numCalls;
		    	}
		    	else {
		    		errs() << "Function is exterally defined\n";
		    	}
		    }
		    else {
		    	errs() << "instruction is not a call instruction\n";
		    }
        }
     }

 	for (Function::iterator it = F.begin(); it != F.end(); it++) {
 		BasicBlock & B = *it;
        for (BasicBlock::iterator b_it = B.begin(); b_it != B.end(); b_it++) {
        	Instruction & I = *b_it;
          if (CallInst * CI = dyn_cast<CallInst>(&I)) {
            // We know we've encountered a call instruction, so we
            // need to determine if it's a call to the
            // function pointed to by m_func or not.
            
            if (numInstr[CI->getCalledFunction()->getName()] < 10) {
	        	llvm::ValueToValueMapTy vmap;
	        	Function * F = CI->getCalledFunction();

				for (Function::iterator it = F->begin(); it != F->end(); it++) {
					BasicBlock & B = *it;
					for (BasicBlock::iterator b_it = B.begin(); b_it != B.end(); b_it++) {
						Instruction & inst = *b_it;
						Instruction * newInst = inst.clone();
						B.getInstList().insert(inst, newInst);
						vmap[&inst] = newInst;
						RemapInstruction(newInst, vmap, RF_NoModuleLevelChanges);							
					}

				}
	        	errs() << "This function would be cloned\n";
	        	errs() << CI->getCalledFunction()->getName();
	        	errs() << "\n";
	        }
          }
        }
     }

     return 0;
}

namespace {

	struct funcInline : public FunctionPass {
		static char ID;
		funcInline() : FunctionPass(ID) {}
		virtual bool runOnFunction(Function & F) {
			errs() << "funcInline: ";
			errs().write_escaped(F.getName()) << '\n';
			startInline(F);
			return false;
		}
	};
}


char funcInline::ID = 0;
static RegisterPass<funcInline> X("funcInline", "Function Inlining Pass", true, false);


/*
void createAndReplace(CallInst* CI,unsigned argIndex){
	Function* callee = CI->getCalledFunction();

	// Replace all uses, using replaceAllUsesWith function
	unsigned IndVar=0;
	// Iterate through all arguments
	for(Function::arg_iterator ai=callee->arg_begin(),aie=callee->arg_end();ai!=aie;++ai){
		// Create Value type
		Value* myconst = CI->getArgOperand(argIndex);
		if(IndVar == argIndex){
			ai->replaceAllUsesWith(myconst);
			IndVar++;
		}
		else {
			IndVar++;
		}
	}
}



// have to iterate through all instructions in callee function, and clone them into the caller function.
void cloneAllInst(BasicBlock* BB, CallInst* CI, Function *callee){	
	// Iterate through all Basic Blocks in a Function
    for (Function::iterator BB=callee->begin(), BBe=callee->end(); BB != BBe; ++BB){
        // Iterate through all Instructions in a BB
        for (BasicBlock::iterator I=BB->begin(),Ie=BB->end();I!=Ie;I++){
        	// Clone instruction
        	auto *clonedInst = I -> clone();
        	// Insert in caller function before calling function
        	BB->getInstList().insert(CI,clonedInst);
        	// Remap instruction
        	llvm::ValueToValueMapTy VMap;
        	llvm::RemapInstruction(clonedInst,VMap,NoModuleLevelChanges = 1)
        }
    }
}
*/

