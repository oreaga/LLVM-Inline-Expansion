//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@//
//                                                                                           //
//                                   ENEE 645 LLVM Project                                   //
//                               function argument Instantiation                             //
//                                                                                           //
//                            members : Eung Joo Lee, Tae Young An                           //
//                                                                                           //
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@//

// To use this pass: 
//                  make install
//                  opt -load opt/lib/A.so -arginst B.bc -o C.bc
//

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Constants.h"
using namespace llvm;

namespace {
    struct ArgInstantiation : public ModulePass {
        static char ID; // Pass identification, replacement for typeid
        ArgInstantiation() : ModulePass(ID) {}
        virtual bool runOnModule(Module &M);
        Function* F_dup;
    };
}

char ArgInstantiation::ID = 0;
static RegisterPass<ArgInstantiation> X("arginst", "Function Arg Instantiation", false, false);

// runOnModule Method.
bool ArgInstantiation::runOnModule(Module &M) {
    // Iterate through all Functions in a module
    for (Module::iterator F=M.begin(),Fe=M.end(); F!=Fe; ++F){
        // Iterate through all Basic Blocks in a Function
        for (Function::iterator BB=F->begin(), BBe=F->end(); BB != BBe; ++BB){
            // Iterate through all Instructions in a BB
            for (BasicBlock::iterator I=BB->begin(),Ie=BB->end();I!=Ie;I++){
                // Check if it is a calling Instruction
                if (CallInst* CI=dyn_cast<CallInst>(I)){
                    Function* callee = CI->getCalledFunction();
                    // Flag to see if this is call site and not a declaration
                    if(callee!=0 && !callee->isDeclaration()){
                        unsigned NumOfArg=CI->getNumArgOperands();
                        unsigned copyInd =0;
                        // Iterate through all actual arguments
                        for(unsigned argIndex=0 ; argIndex < NumOfArg ; ++argIndex){
                            Value* argoperand=CI->getArgOperand(argIndex);
                            // Flag to see if the actual argument has a constant and clone a function
                            if(isa<Constant>(argoperand)){
                                // If the function contains constant arguments, we clone the function just once
                                if(copyInd ==0){
                                    ValueToValueMapTy VMap;
                                    F_dup = CloneFunction(callee,VMap,false);
                                    F_dup->setLinkage(GlobalValue::InternalLinkage);
                                    callee->getParent()->getFunctionList().push_back(F_dup);
                                    CI->setCalledFunction(F_dup);
                                    copyInd++;
                                }

                                //=========================================================================//
                                // We used two different methods to create a constant symbol of type value //
                                //=========================================================================//

                                // Constant symbol (1)=========================================================
                                ConstantInt* cInt = dyn_cast<ConstantInt>(CI->getArgOperand(argIndex));
                                const uint64_t* myconstuint = cInt->getValue().getRawData();
                                Value* myconst = ConstantInt::get(argoperand->getType(), *myconstuint, true);
                                
                                // Constant symbol (2)==========================================================
                                // Value* myconst = CI->getArgOperand(argIndex);
                                
                                unsigned indVar = 0; 
                                // If the formal argument has a constant, we replace it with the value
                                for(Function::arg_iterator ai=F_dup->arg_begin(),aie=F_dup->arg_end();ai!=aie;++ai){
                                    // argIndex is an index number for the actual argument
                                    if(indVar == argIndex){
                                        ai->replaceAllUsesWith(&*myconst);
                                        indVar++;
                                    }
                                    else{
                                        indVar++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
