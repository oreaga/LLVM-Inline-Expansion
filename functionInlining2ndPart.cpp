using namespace llvm;

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