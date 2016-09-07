#ifndef SIMIT_LLVM_DEFINES_H
#define SIMIT_LLVM_DEFINES_H

#if LLVM_MAJOR_VERSION == 3 && LLVM_MINOR_VERSION < 9
namespace llvm {
class ConstantFolder;
template<bool> class IRBuilderDefaultInserter;
template<bool, typename, typename> class IRBuilder;
}
typedef llvm::IRBuilder<true, llvm::ConstantFolder,
                        llvm::IRBuilderDefaultInserter<true>> LLVMIRBuilder;
#else
namespace llvm {
class ConstantFolder;
class IRBuilderDefaultInserter;
template<typename, typename> class IRBuilder;
}
typedef llvm::IRBuilder<llvm::ConstantFolder,
                        llvm::IRBuilderDefaultInserter> LLVMIRBuilder;
#endif

#endif
