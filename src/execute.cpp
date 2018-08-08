#include <LLVMHeaders.h>
#include <llvmEstimator.h>

void runNoArgsFunction(llvm::Function* f, std::unique_ptr<llvm::Module>& module)
{
  auto engine = buildExecutionEngine(module);
  std::vector<llvm::GenericValue> Args(0); // Empty vector as no args are passed
  llvm::GenericValue value = engine->runFunction(f, Args);
}

llvm::ExecutionEngine *buildExecutionEngine(std::unique_ptr<llvm::Module> &module)
{
	std::string collectedErrors;
  llvm::ExecutionEngine *engine =
    llvm::EngineBuilder(std::move(module))
		.setErrorStr(&collectedErrors)
    .setEngineKind(llvm::EngineKind::JIT)
		.create();
		
	if ( !engine )
	{
		std::string error = "Unable to construct execution engine: " + collectedErrors;
		perror(error.c_str());
		return nullptr;
	}
	return engine;
}

llvm::TargetMachine *buildTargetMachine()
{
	std::string error;
  auto targetTriple = llvm::sys::getDefaultTargetTriple();
  auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
	auto cpu = "generic";
	auto features = "";
  auto RM = llvm::Optional<llvm::Reloc::Model>();
  llvm::TargetOptions options;
	if (!target)
	{
    llvm::errs() << error;
		return nullptr;
	}

	return target->createTargetMachine(targetTriple, cpu, features, options, RM);
}

