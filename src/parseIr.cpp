#include <LLVMHeaders.h>

std::unique_ptr<llvm::Module> readIRText(llvm::StringRef text, llvm::LLVMContext& context)
{
  llvm::SMDiagnostic err;
  llvm::MemoryBufferRef buf(text, "IR");
  auto mainModule = llvm::parseIR(buf, err, context);
  llvm::Module* module = mainModule.get();
	if (!module){
    err.print("readIRText", llvm::errs());
	}
  return mainModule;
}

std::unique_ptr<llvm::Module> readIRFile(llvm::StringRef fname, llvm::LLVMContext& context)
{
  llvm::SMDiagnostic err;
  auto mainModule = llvm::parseIRFile(fname, err, context);
  llvm::Module* module = mainModule.get();
	if (!module){
    err.print("readIRFile", llvm::errs());
	}
  return mainModule;
}

