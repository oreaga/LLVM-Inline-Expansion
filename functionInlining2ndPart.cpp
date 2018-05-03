#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <map>
#include <iostream>
using namespace llvm;

bool checkConstant(Value * V) {
	if (isa<Constant>(V)) {
		errs() << "Found constant\n";
		return true;
	}
	else {
		return false;
	}
}

bool checkAllArgsConstant(CallInst * CI) {
	for (int i = 0; i < CI->getNumArgOperands(); i++) {
		Value * arg = CI->getArgOperand(i);
		errs() << *arg;
		errs() << "\n";
		if (!checkConstant(arg)) {
			return false;
		}
	}

	return true;
}

void createAndReplace(CallInst * callee) {
	errs() << "Replacing arguments for: ";
	errs() << callee->getCalledFunction()->getName();
	errs() << "\n";
	LLVMContext & context = callee->getCalledFunction()->getContext();
	IntegerType * i32_type = IntegerType::getInt32Ty(context);
	Function * F = callee->getCalledFunction();

	for (int i = 0; i < callee->getNumArgOperands(); i++) {
		Value * arg = callee->getArgOperand(i);


		if (checkConstant(arg)) {
			errs() << "Found a constant argument\n";
			Constant * argConstant = cast<Constant>(arg);
			errs() << "After cast\n";
			uint64_t argInt = argConstant->getUniqueInteger().getZExtValue();
			errs() << "After int\n";
			ConstantInt * c_int = ConstantInt::get(i32_type, argInt);
			errs() << "After constant int\n";
			errs() << *c_int;
			errs() << "\n";

			int count = 0;
			for (Function::arg_iterator ait = F->arg_begin(); ait != F->arg_end(); ait++) {
				if (count == i) {
					Value & funcArg = *ait;
					funcArg.replaceAllUsesWith(c_int);
				}

				count++;
			}
			errs() << "Done replacing argument\n";
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
		    		errs() << F->getName(); 
		    		numInstr[CI->getCalledFunction()->getName()] = countInstructions(CI->getCalledFunction());
		    		errs() << numInstr[CI->getCalledFunction()->getName()];
			        errs() << "\n";

			    	errs() << "Back in startInline\n";
			    	
			        if (numInstr[CI->getCalledFunction()->getName()] < 10) {
			        	errs() << "beginning recursive call\n";
			        	startInlineCI(CI);
			        }

			        numCalls += 1;
			        errs() << numCalls;
		    	}
		    }
        }
    }

    if (numCalls == 0) {
     	// Call function to replace arguments with constant values
     	errs() << "Reached leaf node";
     	if (checkAllArgsConstant(callI)) {
     		createAndReplace(callI);
     	}
    }
    else {
     	for (Function::iterator it = F.begin(); it != F.end(); it++) {
     		BasicBlock & B = *it;
	        for (BasicBlock::iterator b_it = B.begin(); b_it != B.end(); b_it++) {
	        	Instruction & I = *b_it;
		        if (CallInst * CI = dyn_cast<CallInst>(&I)) {
		            Function * FF = CI->getCalledFunction();

		            if (FF && !FF->isDeclaration()) {
		            	errs() << "Inlining function: ";
		            	errs() << FF->getName();
		            	errs() << "\n";
			            if (numInstr[CI->getCalledFunction()->getName()] < 10 && checkAllArgsConstant(CI)) {
				        	ValueToValueMapTy vmap;

							for (Function::iterator it = FF->begin(); it != FF->end(); it++) {
								BasicBlock & BB = *it;
								for (BasicBlock::iterator b_it = BB.begin(); b_it != BB.end(); b_it++) {
									errs() << "Creating new instruction!!!!!!\n";
									Instruction & inst = *b_it;
									errs() << "Got instruction from iterator\n";
									Instruction * new_inst = inst.clone();
									errs() << "Cloned instruction\n";
									new_inst->insertBefore(&I);
									errs() << "Instruction inserted\n";	
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
			    	
			        if (numInstr[CI->getCalledFunction()->getName()] < 10) {
			        	errs() << "beginning recursive call\n";
			        	startInlineCI(CI);
			        }

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
	            Function * F = CI->getCalledFunction();

	            if (F && !F->isDeclaration() ) {
	            	errs() << "Inlining function: ";
	            	errs() << F->getName();
	            	errs() << "\n";
	            	errs() << (numInstr[CI->getCalledFunction()->getName()] < 10);
	            	errs() << "\n";
	            	errs() << checkAllArgsConstant(CI);
	            	errs() << "\n";
		            if (numInstr[CI->getCalledFunction()->getName()] < 10 && checkAllArgsConstant(CI)) {
			        	llvm::ValueToValueMapTy vmap;

						for (Function::iterator it = F->begin(); it != F->end(); it++) {
							BasicBlock & BB = *it;
							for (BasicBlock::iterator bb_it = BB.begin(); bb_it != BB.end(); bb_it++) {
								Instruction & inst = *bb_it;
								if (ReturnInst * RI = dyn_cast<ReturnInst>(&inst)) {
									Value * retVal = RI->getReturnValue();
									errs() << retVal;
									errs() << "\n";
									errs() << "~~~~~~Return Instruction~~~~~~~: \n";
									errs() << *bb_it;
									errs() << "\n";
									errs() << *CI;
									if (retVal) {
										AllocaInst ai = AllocaInst(retVal->getType());
										ai.insertBefore(CI);
										StoreInst * si = StoreInst(retVal, &ai, CI); 
										CI->replaceAllUsesWith(si->getValueOperand());
									}
									else {
										CI->eraseFromParent();
									}

								}
								else {
									errs() << "Creating new instruction!!!!!!\n";
									Instruction * newInst = inst.clone();
									errs() << *newInst;
									errs() << "**Instruction has been cloned\n";
									newInst->insertBefore(&I);
									errs() << "Instruction has been inserted\n";
									vmap[&inst] = newInst;
									RemapInstruction(newInst, vmap, RF_NoModuleLevelChanges);
								}							
							}

						}
			        	errs() << "This function would be cloned\n";
			        	errs() << CI->getCalledFunction()->getName();
			        	errs() << "\n";
			        	//CI->eraseFromParent();
			        }
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

