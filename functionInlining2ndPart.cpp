#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include <map>
#include <iostream>
using namespace llvm;

int countInstructions(Function &F) {
	int count = 0;

	for (Function::iterator it = F.begin(); it != F.end(); it++) {
		BasicBlock & B = *it;
        for (BasicBlock::iterator b_it = B.begin(); b_it != B.end(); b_it++) {
          count += 1;
        }
    }

    return count;
}

int startInline(Function & F) {
	int numCalls = 0;
	std::map <std::string, int> numInstr;
	// Iterate over each function in the Module and call the replacement functions on it
	for (Function::iterator it = F.begin(); it != F.end(); it++) {
		BasicBlock & B = *it;
        for (BasicBlock::iterator b_it = B.begin(); b_it != B.end(); b_it++) {
        	Instruction & I =  *b_it;
		    if (CallInst * CI = dyn_cast<CallInst>(&I)) {
		        // We know we've encountered a call instruction, so we
		        // need to determine if it's a call to the
		        // function pointed to by m_func or not.
		        numInstr[CI->getCalledFunction()->getName()] = countInstructions(*CI->getCalledFunction());

		        if (numInstr[CI->getCalledFunction()->getName()] < 10) {
		        	startInline(*CI->getCalledFunction());
		        }

		        numCalls += 1;
		    }
        }
     }

     if (numCalls == 0) {
     	// Call function to replace arguments with constant values
     	//createAndReplace(*M);
     	std::cout << "Reached leaf node";
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
	            //cloneAllInst(CallInst);
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
			startInline(F);
			/*
			errs() << "funcInline: ";
			errs().write_escaped(F.getName()) << '\n';
			return false;
			*/
			return false;
		}
	};
}


char funcInline::ID = 0;
static RegisterPass<funcInline> X("funcInline", "Function Inlining Pass", true, false);

/*
void createAndReplace(CallInst* CI,unsigned argIndex){
	Function* callee = CI->getCalledFunction();

	// Create Value type
	Value* myconst = CI->getArgOperand(argIndex);

	// Replace all uses, using replaceAllUsesWith function
	unsigned IndVar=0;
	// Iterate through all arguments
	for(Function::arg_iterator ai=callee->arg_begin(),aie=callee->arg_end();ai!=aie;++ai){
		if(IndVar == argIndex){
			ai->replaceAllUsesWith(myconst);
			IndVar++;
		}
		else{
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
