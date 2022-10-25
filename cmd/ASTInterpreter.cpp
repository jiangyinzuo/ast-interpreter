//==--- tools/clang-check/ClangInterpreter.cpp - Clang Interpreter tool
//--------------===//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;

#include "Environment.h"
#include "InterpreterVisitor.h"

class InterpreterConsumer : public ASTConsumer {
public:
  explicit InterpreterConsumer(const ASTContext &context)
      : mEnv(), mVisitor(context, &mEnv) {}
  virtual ~InterpreterConsumer() {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) override {
    TranslationUnitDecl *decl = Context.getTranslationUnitDecl();
    mEnv.init(decl, &mVisitor);

    FunctionDecl *entry = mEnv.getEntry();
    mVisitor.VisitStmt(entry->getBody());
    auto regRet = mEnv.getMainRet();
    if (regRet != 0) {
      llvm::dbgs() << "main returns " << regRet << "\n";
    }
  }

private:
  Environment mEnv;
  InterpreterVisitor mVisitor;
};

class InterpreterClassAction : public ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new InterpreterConsumer(Compiler.getASTContext()));
  }
};

int main(int argc, char **argv) {
  if (argc > 1) {
    clang::tooling::runToolOnCode(
        std::unique_ptr<clang::FrontendAction>(new InterpreterClassAction),
        argv[1]);
  }
  return 0;
}
