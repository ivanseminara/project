////////////////////////////////////////////////////////////////////////////////
//
//                     Tagged Memory Pass
//                  Author: Ivan R. Seminara
//
//
//
////////////////////////////////////////////////////////////////////////////////

#define DEBUG_TYPE "irs_tagged_memory"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/ConstantFolder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"


#include <vector>


using namespace llvm;
using std::vector;

namespace {

    typedef Module::FunctionListType&               FList;
    typedef Module::FunctionListType::iterator      FElement;
    typedef Function::BasicBlockListType&           BList;
    typedef Function::BasicBlockListType::iterator  BElement;
    typedef BasicBlock::InstListType&               IList;
    typedef BasicBlock::InstListType::iterator      IElement;

    const char* TypeNames[17] = {"Void", "Half", "Float", "Double", "x86_FP80",
        "FP128", "PPC_FP128", "Label", "Metadata", 
        "x86_MMX", "Token", "Integer", "Function", 
        "Struct", "Array", "Pointer", "Vector"};

    struct IRSTaggedMemory : public ModulePass {

        static char ID; 

        LLVMContext* Context;
        Module* LocalModule;
        Function* CurrentFunction;
        Function* FunctionTagCheckFailed = NULL;
        ConstantFolder Folder; 

        IRSTaggedMemory() : ModulePass(ID) {}

        //////////////////////////////////////////////////////////////////////////////
        ///
        // The following section is a straightforward copy of Matthew Toseland's code. 
        // It sets up the trap handlers for a tag failure. 
        //
        ///////////////////////////////////////////////////////////////////////////////
        static const long LD_TAG_CSR_VALUE = (1 << IRBuilderBase::TAG_WRITE_ONLY) | (1 << IRBuilderBase::TAG_INVALID);
        static const long SD_TAG_CSR_VALUE = (1 << IRBuilderBase::TAG_READ_ONLY) | (1 << IRBuilderBase::TAG_INVALID);

        /// \brief Get a constant 64-bit value.
        ConstantInt *getInt64(uint64_t C, LLVMContext &Context) {
            return ConstantInt::get(Type::getInt64Ty(Context), C);
        }

        //  It inserts the handler for a tag failure. 
        void addSetupCSRs(Module &M, LLVMContext &Context) {
            // FIXME This is yet another gross hack. :)
            // This will add one global init per module.
            // It is not clear who will eventually be responsible for setting 
            //  the LDCT/SDCT CSRs:
            // Will they be settable from userspace?
            // The kernel could enforce a single global policy, or it could save 
            //  and restore values for each process.
            // FIXME In any case they should be set once per process, not once per module!
            // Fixing this requires support in the linker or the frontend.
            AttributeSet fnAttributes;
            Function *f = M.getFunction("__llvm_riscv_init_tagged_memory_csrs");
            if(f) {
                return;
            }
            FunctionType *type = FunctionType::get(Type::getVoidTy(Context),
                    false);
            f = Function::Create(type, GlobalValue::LinkOnceAnyLinkage, // Allow overriding!
                    "__llvm_riscv_init_tagged_memory_csrs", &M);
            BasicBlock* entry = BasicBlock::Create(Context, "entry", f);
            IRBuilder<> builder(entry);
            Value *TheFn = Intrinsic::getDeclaration(&M, Intrinsic::riscv_set_tm_trap_ld);
            errs() << "Function is " << *TheFn << "\n";
            builder.CreateCall(TheFn, getInt64(LD_TAG_CSR_VALUE, Context));
            TheFn = Intrinsic::getDeclaration(&M, Intrinsic::riscv_set_tm_trap_sd);
            errs() << "Function is " << *TheFn << "\n";
            builder.CreateCall(TheFn, getInt64(SD_TAG_CSR_VALUE, Context));
            builder.CreateRetVoid();
            appendToGlobalCtors(M, f, 0);
        }

        // This is a straightforward copy of Matthew's code.
        // This creates the function called when a tag comparison fails.
        // It takes no arguments, and returns nothing, it will simply generate a trap.
        /*        void insertAbortFunction() {
                  FunctionType* type = FunctionType::get(Type::getVoidTy(*Context), false);
                  Function* function = Function::Create(type, GlobalValue::LinkOnceAnyLinkage, 
                  "tag_failure", LocalModule);
                  function->addFnAttr(Attribute::NoReturn);
                  BasicBlock* entry = BasicBlock::Create(*Context, "entry", function);
                  IRBuilder<> builder(entry);
                  builder.CreateTrap();
                  builder.CreateRetVoid();
                  tag_failure = function;
                  }
                  */
        bool addCheckTaggedFailed(Module &M, LLVMContext &Context) {
            AttributeSet fnAttributes;
            Function *f = M.getFunction("__llvm_riscv_check_tagged_failure");
            if(f) {
                FunctionTagCheckFailed = f;
                return f;
            }
            FunctionType *type = FunctionType::get(Type::getVoidTy(Context),
                    false);
            f = Function::Create(type, GlobalValue::LinkOnceAnyLinkage, // Allow overriding!
                    "__llvm_riscv_check_tagged_failure", &M);
            f -> addFnAttr(Attribute::NoReturn);
            BasicBlock* entry = BasicBlock::Create(Context, "entry", f);
            IRBuilder<> builder(entry);
            builder.CreateTrap();
            builder.CreateRetVoid(); // FIXME Needs a terminal?
            FunctionTagCheckFailed = f;
            return true;
        }
        ////////////////////////////////////////////////////////////////////////////////
        // End of Matthew Toseland's section
        ////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////////////
        // 
        // Iterate over the blocks and find all the pointers to function. 
        // Return the load and store instructions for later rewriting.
        //
        ////////////////////////////////////////////////////////////////////////////////
        void fetch(BList blocks, std::vector<StoreInst*>* stores, std::vector<LoadInst*>* loads) {
            for(BElement block = blocks.begin(); block != blocks.end(); block++) {
                IList instructions = block->getInstList();
                for(IElement instruction = instructions.begin(); instruction != instructions.end(); instruction++) {
                    if(pointsToFunction(instruction)) {
                        if(isa<StoreInst>(instruction)) {
                            StoreInst& store = (StoreInst&)*instruction;
                            stores->push_back(&store);
                        }
                        if(isa<LoadInst>(instruction)) {
                            LoadInst& load = (LoadInst&)*instruction;
                            loads->push_back(&load);
                        }
                    }
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        // 
        // Determine whether the instruction, either a load or store, points to
        //  a function.
        //
        ////////////////////////////////////////////////////////////////////////////////
        bool pointsToFunction(IElement instruction) {
            Value* pointer;
            Type* type, *points_to;

            if(isa<LoadInst>(instruction)) {
                LoadInst& load = (LoadInst&)*instruction;
                pointer = load.getPointerOperand();
            } else if(isa<StoreInst>(instruction)) {
                StoreInst& store = (StoreInst&)*instruction;
                pointer = store.getPointerOperand();
            } else 
                return false;

            type = pointer->getType();
            points_to = getTargetType(type);
            return isa<FunctionType>(points_to);
        }

        ////////////////////////////////////////////////////////////////////////////////
        // 
        // Main handle. Insert some required support functions in the Module then 
        //  iterate over the functions looking for load/store to tag.
        //
        ////////////////////////////////////////////////////////////////////////////////
        virtual bool runOnModule(Module& M) {
            // Init globals
            Context = &M.getContext();
            LocalModule = &M;
            bool rewriting_done = false;   // track if any work has been done
#ifdef DEBUG_TYPE
            errs() << "Examining Module: \n";
#endif
            // Call Matthew's function to add the tagged memory failure handler
            LLVMContext &C = M.getContext();
            addSetupCSRs(M, C);
            addCheckTaggedFailed(M, C);

            FList functions = M.getFunctionList();

            bool tmp_rewrite; 
            for(FElement function = functions.begin(); function != functions.end(); function++) {
                // Don't rewrite the functions we have added.
                if(function->getName() != "llvm.riscv.store.tagged" ||
                   function->getName() != "llvm.riscv.load.tagged" ||
                   function->getName() != "llvm.riscv.set.tm.trap" ||
                   function->getName() != "__llvm_riscv_check_tagged_failure") {

                    tmp_rewrite = processFunction(function);
                    if(!rewriting_done) rewriting_done = tmp_rewrite;
                }
            }
            return rewriting_done;  
        }

        ////////////////////////////////////////////////////////////////////////////////
        // 
        // Iterate over the Basic Blocks of a function looking for load and stores 
        //  of function pointers (rewriting a list on the fly tends to backfire...).
        //  Having gathered them replace them with tagged equivalents.
        //
        ////////////////////////////////////////////////////////////////////////////////
        bool processFunction(Function* function) {
#ifdef DEBUG_TYPE
            errs() << " Function: " << function->getName() << "\n";
#endif
            CurrentFunction = function;
            BList blocks = function->getBasicBlockList();
            std::vector<StoreInst*>* stores = new std::vector<StoreInst*>();
            std::vector<LoadInst*>* loads = new std::vector<LoadInst*>();

            fetch(blocks, stores, loads);

#ifdef DEBUG_TYPE
            errs() << "Found " << stores->size() << " stores.\n";
            errs() << "Found " << loads->size() << " loads.\n";
#endif
            bool rewrites = false;

            for(vector<StoreInst*>::iterator store = stores->begin(); store != stores->end(); store++) {
                rewrites = true;
                tagStore(*store);
            }

            for(vector<LoadInst*>::iterator load = loads->begin(); load != loads->end(); load++) {
                rewrites = true;
                tagLoad(*load);
            }

            CurrentFunction = NULL;
            return rewrites;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // 
        // Tag a load instruction. 
        // It expects a pointer to function and will replace the load with the 
        //  intrinsic riscv_load_tagged.
        //
        ////////////////////////////////////////////////////////////////////////////////
        bool tagLoad(LoadInst* instruction) {
            Value* pointer, *value = NULL;
            Type* type, *points_to;

            // Just get some info out of the instruction
            pointer = instruction->getPointerOperand();
            type = pointer->getType();
            points_to = ((SequentialType*)type)->getElementType();
            IElement load_position(*instruction);

#ifdef DEBUG_TYPE
            errs() << "\nTagging: " << *instruction << "\n";
            printInstruction(instruction, pointer, value, type, points_to);
#endif

            // We will need access to the list to insert instructions
            BasicBlock::InstListType& instructions = instruction->getParent()->getInstList(); 

            // First get the Type of the pointer to adapt it to the function signature,
            //  according to Matthew this is likely to change in the future.
            PointerType* ptype = cast<PointerType>(type);

            // Create a cast
            ptype = Type::getInt8PtrTy(*Context, ptype->getAddressSpace());
            BitCastInst* cast_to_int = new BitCastInst(pointer, ptype, "cast_to_int8_pointer");

            // and insert it in the instruction stream
            instructions.insert(instruction, cast_to_int);
            pointer = cast_to_int;

            // Prepare the argument to the intrinsic
            Value* args[] = {
                pointer
            };

            // Get the tagged load instruction 
            Value* intr_load = Intrinsic::getDeclaration(LocalModule, Intrinsic::riscv_load_tagged);

            // Create a call to it with the required arguments
            CallInst* tagged_load = CallInst::Create(intr_load, args, "", instruction);

            // Tag and pointer are stored together, we need to get them separately.
            //  Look at IntrinsicsRISCV.td for (very little) additional detail.
            unsigned arg1[] = { 1 };
            unsigned arg0[] = { 0 };
            Instruction* tag       = ExtractValueInst::Create(tagged_load, arg1, "extract_tag", instruction);
            Instruction* ptr_value = ExtractValueInst::Create(tagged_load, arg0, "extract_pointer", instruction);

            // Insert a compare instruction to check that the tag has not changed.
            Instruction* compare_tags = new ICmpInst(
                    instruction,                                 // Insert before this
                    ICmpInst::ICMP_NE,                          // Predicate
                    tag,                                        // LHS
                    ConstantInt::get(Type::getInt64Ty(*Context),// RHS
                        IRBuilderBase::TAG_CLEAN_FPTR)
                    );

            Value* real_pointer;
            if(ptr_value->getType() == points_to) {
                real_pointer = value;
            } else if(Constant* constant_cast = dyn_cast<Constant>(ptr_value)) {
                real_pointer = Folder.CreateCast(Instruction::PtrToInt, constant_cast, Type::getInt64Ty(*Context));
            } else {
                Instruction* cast_int_to_ptr = CastInst::Create(Instruction::IntToPtr, ptr_value, points_to);
                instructions.insert(instruction, cast_int_to_ptr);
                real_pointer = cast_int_to_ptr;
            }

            BasicBlock::InstListType::iterator insert(instruction);

#ifdef DEBUG_TYPE
            errs() << "-Rewritten Load-------------------------------------------\n";
            for(BasicBlock::InstListType::iterator iter = instructions.begin(); iter != instructions.end(); iter++)
                errs() << *iter << "\n";
            errs() << "----------------------------------------------------------\n";
#endif
            ReplaceInstWithValue(instructions, insert, real_pointer);

            // Time to compare the loaded tag with the one we expected. If they don't match
            //  we need to abort, otherwise the program can continue.
            // To do this we will insert a compare-and-branch(well, at least a branch after 
            //  the compare we already have), it will also require the introduction of a 
            //  new BasicBlock: execution will jump to it if the tag is corrupted. 
            //  It will simply abort; we are waiting for support from the processor.

            // Get a pointer to the compare
            Instruction* pivot = compare_tags->getNextNode();
            // The part before the load
            BasicBlock* Root = pivot->getParent();

            // Execution will continue here if the tags match
            BasicBlock* Match = Root->splitBasicBlock(pivot);

            // We will need the terminator later.
            // All Blocks MUST end with a terminator, a call doesn't qualify. There is no
            //  warranty that separate blocks will be chained as expected, compilation can,
            //  and most likely will, reorder them. If the terminator is missing the Validator
            //  will fail with an (incomprehensible) error.
            TerminatorInst* root_terminator = Root->getTerminator();

            // Create the error handling Block. It's empty for now.
            BasicBlock* Mismatch = BasicBlock::Create(
                    *Context,            // Context
                    "",                 // Name
                    CurrentFunction,   // Parent
                    Match              // Insert before
                    );

            // Create the branch instruction, bind it to a comparison and assign the jump targets
            BranchInst* branch_on_tags = BranchInst::Create(
                    Mismatch,       // If True
                    Match,          // If False
                    compare_tags    // Condition
                    );

            // Now the Root Block ends with a branch
            ReplaceInstWithInst(root_terminator, branch_on_tags);

            // Add a dummy terminator to the block. This is required.
            TerminatorInst* mismatch_terminator = Mismatch->getTerminator();
            mismatch_terminator = new UnreachableInst(*Context, Mismatch);

            // Append a call to FunctionTagCheckFailed to the block
            CallInst::Create(FunctionTagCheckFailed, ArrayRef<Value*>(), "", mismatch_terminator);
            // Update the reference to the Root block
            Root = compare_tags->getParent(); 

            return true;
        }

        ////////////////////////////////////////////////////////////////////////////////
        // 
        // Tag a store instruction. 
        // It expects a pointer to function and will replace the store with the 
        //  intrinsic riscv_store_tagged.
        //
        ////////////////////////////////////////////////////////////////////////////////
        bool tagStore(StoreInst* instruction) { 
            errs() << "Entering tagStore\n";
            Value* pointer, *value;
            Type* type, *points_to;

            pointer = instruction->getPointerOperand();
            value = instruction->getValueOperand();
            type = pointer->getType();
            points_to = getTargetType(type);

#ifdef DEBUG_TYPE
            printInstruction(instruction, pointer, value, type, points_to);
#endif
            BasicBlock::InstListType& instructions = instruction->getParent()->getInstList(); 

            // First get the Type of the pointer to adapt it to the function signature,
            //  according to Matthew this is likely to change in the future.
            PointerType* ptype = cast<PointerType>(type);

            // Create the cast
            ptype = Type::getInt8PtrTy(*Context, ptype->getAddressSpace());
            BitCastInst* cast_to_int = new BitCastInst(pointer, ptype, "cast_to_int8_pointer");

            // and insert it in the instruction stream
            instructions.insert(instruction, cast_to_int);
            pointer = cast_to_int;

            // Now we cast the value to store in the pointer and add the cast instruction 
            Value* int_pointer;

            if(value->getType() == points_to) {
                int_pointer = value;
            } else if (Constant* constant_cast = dyn_cast<Constant>(value)) {
                int_pointer = Folder.CreateCast(Instruction::PtrToInt, constant_cast, Type::getInt64Ty(*Context));
            } else { 
                // Cast required: create cast instruction
                Instruction* cast_ptr_to_int = CastInst::Create(Instruction::PtrToInt, value, Type::getInt64Ty(*Context));
                //  and insert it before the store
                instructions.insert(instruction, cast_ptr_to_int);
                int_pointer = cast_ptr_to_int;
            }

            // This is the tag we will store with the pointer (see IRBuilder.h)
            Value* tag = ConstantInt::get(Type::getInt64Ty(*Context), IRBuilderBase::TAG_CLEAN_FPTR);
            // The riscv_store_tagged intrinsic takes the following arguments(see IntrinsicsRISCV.td):
            Value* args[] = {
                int_pointer,    // Value to write
                tag,            // Tag
                pointer         // Pointer to write to
            };
            // Get the tagged store instruction 
            Value* intr_store = Intrinsic::getDeclaration(LocalModule, Intrinsic::riscv_store_tagged);
            // Create a call to it with the required arguments
            CallInst* tagged_store = CallInst::Create(intr_store, args, "");
            // Replace the regular store with the tagged one in the Block
            IElement instruction_iter(instruction);
            ReplaceInstWithInst(instructions, instruction_iter, tagged_store);

#ifdef DEBUG_TYPE
            errs() << "-Rewritten Store------------------------------------------\n";
            for(BasicBlock::InstListType::iterator iter = instructions.begin(); iter != instructions.end(); iter++)
                errs() << *iter << "\n";
            errs() << "----------------------------------------------------------\n";
#endif

            return true;
        }

        Type* getTargetType(Type* pointer) {
            Type* target = ((SequentialType*)pointer)->getElementType();
            return isa<PointerType>(target) ? getTargetType(target) : target;
        }

        void printInstruction(IElement instruction, Value* pointer, Value* value, Type* type, Type* points_to) {
            errs() << instruction->getOpcodeName() << " instruction:\n";
            errs() << "   - value: " << value << "\n";
            errs() << "   - type: " << TypeNames[type->getTypeID()] << "[" << *type << "]\n";
            errs() << "   - target type: "<< *points_to << "\n";
        }

        };

        char IRSTaggedMemory::ID = 0;
        static RegisterPass<IRSTaggedMemory> X("irs_tagged_memory", "Experimental Memory Tagging Pass");

    } //namespace
