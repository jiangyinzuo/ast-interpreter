#include "Environment.h"
#include "InterpreterVisitor.h"
#include "Object.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include <cstdlib>
#include <memory>
#include <vector>

static std::string getDeclName(Decl *decl) {
  std::string name;
  if (VarDecl *vardecl = dyn_cast<VarDecl>(decl)) {
    name = vardecl->getName().str();
  } else if (FunctionDecl *fdecl = dyn_cast<FunctionDecl>(fdecl)) {
    name = fdecl->getName().str();
  } else {
    llvm::errs() << "unimplemented getDeclName: " << decl->getDeclKindName()
                 << '\n';
    exit(-1);
  }
  return name;
}

void StackFrame::bindDecl(Decl *decl, MappedValue val) {
  assert(val != nullptr);
  mVars[decl] = std::move(val);
}

Object *StackFrame::getDeclVal(std::vector<StackFrame> &stack, Decl *name) {
  auto *curFrame = this;
  for (;;) {
    auto result = curFrame->mVars.find(name);
    if (result != curFrame->mVars.end()) {
      return result->second.get();
    }
    if (curFrame->mFatherID == kNoFather) {
      llvm::errs() << "no decl";
      exit(-1);
    }
    curFrame = &stack[curFrame->mFatherID];
  }
}

void Environment::init(TranslationUnitDecl *unit, InterpreterVisitor *visitor) {
  mVisitor = visitor;
  mStack.push_back(std::move(StackFrame(StackFrame::kNoFather)));
  StackFrame mainStackFrame(0);
  for (auto i = unit->decls_begin(), e = unit->decls_end(); i != e; ++i) {
    if (FunctionDecl *fdecl = dyn_cast<FunctionDecl>(*i)) {
      mStack.back().bindDecl(fdecl, std::make_unique<Function>(fdecl));
      if (fdecl->getName().equals("FREE"))
        mFree = fdecl;
      else if (fdecl->getName().equals("MALLOC"))
        mMalloc = fdecl;
      else if (fdecl->getName().equals("GET"))
        mInput = fdecl;
      else if (fdecl->getName().equals("PRINT"))
        mOutput = fdecl;
      else if (fdecl->getName().equals("main")) {
        mEntry = fdecl;
        // bind main's parameters
        for (auto *it = fdecl->param_begin(); it != fdecl->param_end(); ++it) {
          mainStackFrame.bindDecl(
              *it, std::make_unique<Value>(0L)); // TODO: decl value
        }
      } else {
        // user defined function
      }
    } else if (VarDecl *vardecl = dyn_cast<VarDecl>(*i)) {
      Expr *init_expr = vardecl->getInit();
      auto tp = vardecl->getType();
      if (tp->isIntegerType() || tp->isCharType()) {
        if (init_expr == nullptr) {
          mStack.back().bindDecl(vardecl, std::make_unique<Value>(0L));
        } else if (IntegerLiteral *int_lit =
                       dyn_cast<IntegerLiteral>(init_expr)) {
          mStack.back().bindDecl(vardecl,
                                 std::make_unique<Value>(
                                     (long)int_lit->getValue().getSExtValue()));
        } else if (CharacterLiteral *char_lit =
                       dyn_cast<CharacterLiteral>(init_expr)) {
          mStack.back().bindDecl(
              vardecl, std::make_unique<Value>(
                           static_cast<long>((char_lit->getValue()))));
        } else {
          llvm::errs() << "unimplement literal: "
                       << init_expr->getStmtClassName();
        }
      } else if (tp->isPointerType()) {
        // TODO: init
      } else if (tp->isConstantArrayType() && tp->isConstantSizeType()) {
        arrayType(vardecl, init_expr, tp);
      }
    } else if (TypedefDecl *typeDefDecl = dyn_cast<TypedefDecl>(*i)) {
      // do nothing
    } else {
      llvm::errs() << "unimplement decl: " << i->getDeclKindName();
      exit(-1);
    }
  }
  mStack.push_back(std::move(mainStackFrame));
}

void Environment::intLiteral(IntegerLiteral *int_lit) {
  mStack.back().setPC(int_lit);
  mStack.back().bindStmt(int_lit, std::make_unique<Value>(static_cast<long>(
                                      int_lit->getValue().getSExtValue())));
}

void Environment::charLiteral(CharacterLiteral *char_lit) {
  mStack.back().setPC(char_lit);
  mStack.back().bindStmt(char_lit, std::make_unique<Value>(static_cast<long>(
                                       char_lit->getValue())));
}

void Environment::binop(BinaryOperator *bop) {
  Expr *left = bop->getLHS();
  Expr *right = bop->getRHS();
  switch (bop->getOpcode()) {
  case clang::BO_Assign: {
    auto rvalue = mStack.back().getStmtVal(right);
    auto lvalue = mStack.back().getStmtVal(left);
    lvalue->AssignObj(*rvalue);
    mStack.back().bindStmt(left, rvalue);
    break;
  }
  case clang::BO_Add: {
    auto left_value = mStack.back().getStmtVal(left);
    auto right_value = mStack.back().getStmtVal(right);
    mStack.back().bindStmt(bop, *left_value + *right_value);
    break;
  }
  case clang::BO_Sub: {
    auto left_value = mStack.back().getStmtVal(left);
    auto right_value = mStack.back().getStmtVal(right);
    mStack.back().bindStmt(bop, *left_value - *right_value);
    break;
  }
  case clang::BO_Mul: {
    auto left_value = mStack.back().getStmtVal(left);
    auto right_value = mStack.back().getStmtVal(right);
    mStack.back().bindStmt(bop, *left_value * *right_value);
    break;
  }
  case clang::BO_Div: {
    auto left_value = mStack.back().getStmtVal(left);
    auto right_value = mStack.back().getStmtVal(right);
    mStack.back().bindStmt(bop, *left_value / *right_value);
    break;
  }
  default: {
    llvm::errs() << "unimplemented binop "
                 << BinaryOperator::getOpcodeStr(bop->getOpcode()) << '\n';
    exit(-1);
  }
  }
}

void Environment::unary(UnaryOperator *uop) {
  auto value = mStack.back().getStmtVal(uop->getSubExpr());
  switch (uop->getOpcode()) {
  case clang::UO_Plus: {
    mStack.back().bindStmt(uop, value);
    break;
  }
  case clang::UO_Minus: {
    mStack.back().bindStmt(uop, -*value);
    break;
  }
  case clang::UO_Deref: {
    mStack.back().bindStmt(uop, value->DerefObj());
    break;
  }
  default: {
    llvm::errs() << "unimplemented unary operator"
                 << UnaryOperator::getOpcodeStr(uop->getOpcode()) << '\n';
    exit(-1);
  }
  }
}

void Environment::unaryOrTypeTrait(UnaryExprOrTypeTraitExpr *expr) {
  auto ty = expr->getTypeOfArgument();
  // TODO: modify the hacked code
  if (ty->isIntegerType()) {
    mStack.back().bindStmt(expr, std::make_unique<Value>(4L));
  } else if (ty->isPointerType()) {
    mStack.back().bindStmt(expr, std::make_unique<Value>(8L));
  } else {
    llvm::errs() << "unimplemented unaryOrTypeTrait"
                 << "\n";
    exit(-1);
  }
}

void Environment::decl(DeclStmt *declstmt) {
  for (DeclStmt::decl_iterator it = declstmt->decl_begin(),
                               ie = declstmt->decl_end();
       it != ie; ++it) {
    Decl *decl = *it;
    if (VarDecl *vardecl = dyn_cast<VarDecl>(decl)) {
      auto varDeclType = vardecl->getType();
      Expr *init_expr = vardecl->getInit();
      if (varDeclType->isConstantArrayType() &&
          varDeclType->isConstantSizeType()) {
        arrayType(vardecl, init_expr, varDeclType);
      } else if (varDeclType->isIntegerType() || varDeclType->isCharType()) {
        if (init_expr == nullptr) {
          mStack.back().bindDecl(vardecl, std::make_unique<Value>(0L));
        } else {
          auto init_value = mStack.back().getStmtVal(init_expr);
          std::unique_ptr<Value> v = std::make_unique<Value>(0L);
          v->AssignObj(*init_value);
          mStack.back().bindDecl(vardecl, std::move(v));
        }
      } else if (varDeclType->isPointerType()) {
        if (init_expr == nullptr) {
          mStack.back().bindDecl(vardecl, std::make_unique<Pointer>(nullptr));
        } else {
          auto init_value = mStack.back().getStmtVal(init_expr);
          std::unique_ptr<Pointer> v = std::make_unique<Pointer>(nullptr);
          v->AssignObj(*init_value);
          mStack.back().bindDecl(vardecl, std::move(v));
        }
      } else {
        llvm::errs() << "unimplemented vardecl \n";
        exit(-1);
      }
    } else {
      llvm::errs() << "not vardecl\n";
      exit(-1);
    }
  }
}

void Environment::declref(DeclRefExpr *declref) {
  mStack.back().setPC(declref);
  auto declrefType = declref->getType();
  if (declrefType->isIntegerType() || declrefType->isCharType() ||
      declrefType->isArrayType() || declrefType->isFunctionType() ||
      declrefType->isPointerType()) {
    llvm::dbgs() << declref->getDecl()->getDeclName().getAsString() << '\n';
    Decl *decl = declref->getFoundDecl();
    auto val = mStack.back().getDeclVal(mStack, decl);
    mStack.back().bindStmt(declref, val);
  } else {
    llvm::errs() << "unimplement declref type. name: "
                 << declref->getDecl()->getName()
                 << ", classname: " << declrefType->getTypeClassName() << '\n';
    exit(-1);
  }
}

void Environment::paren(ParenExpr *expr) {
  auto obj = mStack.back().getStmtVal(expr->getSubExpr());
  mStack.back().bindStmt(expr, obj);
}

/// !TODO Support Function Call
void Environment::call(CallExpr *callexpr) {
  mStack.back().setPC(callexpr);
  FunctionDecl *callee = callexpr->getDirectCallee();
  if (callee == mInput) {
    long val;
    llvm::errs() << "Please Input an Integer Value : ";
    scanf("%ld", &val);

    mStack.back().bindStmt(callexpr, std::make_unique<Value>(val));
  } else if (callee == mOutput) {
    Expr *decl = callexpr->getArg(0);
    auto val = mStack.back().getStmtVal(decl);
    llvm::errs() << val->GetValueObj();
  } else if (callee == mMalloc) {
    Expr *decl = callexpr->getArg(0);
    auto val = mStack.back().getStmtVal(decl);
    auto arr = std::make_unique<Array>(val->GetValueObj());
    auto ptr = std::make_unique<Pointer>(arr->GetPtr());
    mHeap[ptr->GetValue()] = std::move(arr);
    mStack.back().bindStmt(callexpr, std::move(ptr));
  } else if (callee == mFree) {
    Expr *decl = callexpr->getArg(0);
    auto val = mStack.back().getStmtVal(decl);
    int res = mHeap.erase(val->GetValueObj());
    assert(res == 1);
  } else {
    //  call user-defined function
    if (callee->getNumParams() != callexpr->getNumArgs()) {
      llvm::errs() << "expected " << callee->getNumParams() << "args, actual "
                   << callexpr->getNumArgs() << '\n';
      exit(-1);
    }
    // prepare StackFrame
    StackFrame stack_frame(0);
    CallExpr::arg_iterator arg;
    FunctionDecl::param_iterator param;
    for (arg = callexpr->arg_begin(), param = callee->param_begin();
         arg != callexpr->arg_end() && param != callee->param_end();
         ++arg, ++param) {
      auto val = mStack.back().getStmtVal(*arg);
      std::unique_ptr<Value> v = std::make_unique<Value>(0L);
      v->AssignObj(*val);
      stack_frame.bindDecl(*param, std::move(v));
    }
    mStack.push_back(std::move(stack_frame));
    mVisitor->VisitStmt(callee->getBody());

    // resume PC
    auto ret = mStack.back().MoveRetReg();
    mStack.pop_back();
    mStack.back().bindStmt(callexpr, std::move(ret));
  }
}

void Environment::implicitCast(ImplicitCastExpr *expr) {
  auto obj = mStack.back().getStmtVal(expr->getSubExpr());
  mStack.back().bindStmt(expr, obj);
}

void Environment::cast(CastExpr *expr) {
  auto obj = mStack.back().getStmtVal(expr->getSubExpr());
  mStack.back().bindStmt(expr, obj);
}

void Environment::arraySubscript(ArraySubscriptExpr *arrSubExpr) {
  auto idx = mStack.back().getStmtVal(arrSubExpr->getIdx());
  Expr *baseExpr = arrSubExpr->getBase();
  auto arr = mStack.back().getStmtVal(baseExpr);
  mStack.back().bindStmt(arrSubExpr, (*arr)[*idx]);
}

void Environment::compoundStmtBegin(CompoundStmt *stmt) {
  mStack.back().setPC(stmt);
  if (mNextCompoundStmtAddScope) {
    mStack.push_back(std::move(StackFrame(mStack.size() - 1)));
  }
}
void Environment::compoundStmtEnd() { mStack.pop_back(); }

void Environment::returnStmt(ReturnStmt *stmt) {
  Expr *e = stmt->getRetValue();
  if (e != nullptr) {
    std::unique_ptr<Object> ret = mStack.back().MoveStmtVal(e);
    mStack.back().setRetReg(std::move(ret));
  }
}

void Environment::arrayType(VarDecl *vardecl, Expr *init_expr,
                            clang::QualType tp) {
  auto array_tp = dyn_cast<ConstantArrayType>(tp);
  if (init_expr == nullptr) {
    mStack.back().bindDecl(
        vardecl, std::make_unique<Array>(array_tp->getSize().getZExtValue()));
  } else {
    llvm::errs() << "unimplement array initialization.\n";
    exit(-1);
  }
}