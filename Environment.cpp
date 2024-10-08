#include "Environment.h"
#include "InterpreterVisitor.h"
#include "ObjectV2.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include <cstdlib>
#include <memory>

const int StackFrame::kNoFather = -1;
static unsigned getPointerType(QualType ty) {
  unsigned pointerType = 0;
  while (const PointerType *pt = dyn_cast<PointerType>(ty.getTypePtr())) {
    ty = pt->getPointeeType();
    ++pointerType;
  }
  return pointerType;
}

StackFrame::~StackFrame() {
  if (!mArrs.empty()) {
    // llvm::dbgs() << "destroy stack frame's " << mArrs.size() << " arrays\n";
    for (long *p : mArrs) {
      delete[] p;
    }
    mArrs.clear();
  }
}

void StackFrame::bindDecl(Decl *decl, ObjectV2 val) { mVars[decl] = (val); }

ObjectV2 StackFrame::getDeclValRef(std::deque<StackFrame> &stack, Decl *name) {
#ifndef NDEBUG
  std::deque<int> stackIDs;
  stackIDs.push_back(stack.size() - 1);
#endif
  // llvm::dbgs() << "ID=" << name->getID()
  //             << "kindname= " << name->getDeclKindName() << '\n';
  auto *curFrame = this;
  for (;;) {
    auto result = curFrame->mVars.find(name);
    if (result != curFrame->mVars.end()) {
      return result->second.LValueRef();
    }
    if (curFrame->mFatherID == kNoFather) {
      llvm::errs() << "no decl " << name->getDeclKindName() << '\n';
#ifndef NDEBUG
      for (int id : stackIDs) {
        llvm::errs() << id << ' ';
      }
      llvm::errs() << "ID=" << name->getID() << '\n';
#endif
      exit(-1);
    }
#ifndef NDEBUG
    stackIDs.push_back(curFrame->mFatherID);
#endif
    curFrame = &stack[curFrame->mFatherID];
  }
}

void Environment::init(TranslationUnitDecl *unit, InterpreterVisitor *visitor) {
  mVisitor = visitor;
  mStack.emplace_back(StackFrame::kNoFather);
  StackFrame mainStackFrame(0);
  for (auto i = unit->decls_begin(), e = unit->decls_end(); i != e; ++i) {
    if (FunctionDecl *fdecl = dyn_cast<FunctionDecl>(*i)) {
      mStack.back().bindDecl(fdecl, ObjectV2(0, 0, (long)fdecl));
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
          unsigned pointerType = getPointerType((*it)->getType());
          mainStackFrame.bindDecl(
              *it, ObjectV2(pointerType, 0, 0L)); // TODO: decl value
        }
      }
      // skip user defined function
    } else if (VarDecl *vardecl = dyn_cast<VarDecl>(*i)) {
      Expr *init_expr = vardecl->getInit();
      QualType tp = vardecl->getType();
      if (tp->isIntegerType() || tp->isCharType()) {
        if (init_expr == nullptr) {
          mStack.back().bindDecl(vardecl, ObjectV2(0, 0, 0L));
        } else if (IntegerLiteral *int_lit =
                       dyn_cast<IntegerLiteral>(init_expr)) {
          mStack.back().bindDecl(
              vardecl,
              ObjectV2(0, 0, (long)int_lit->getValue().getSExtValue()));
        } else if (CharacterLiteral *char_lit =
                       dyn_cast<CharacterLiteral>(init_expr)) {
          mStack.back().bindDecl(
              vardecl,
              ObjectV2(0, 0, static_cast<long>((char_lit->getValue()))));
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
  mStack.back().bindStmt(
      int_lit,
      ObjectV2(0, 0, static_cast<long>(int_lit->getValue().getSExtValue())));
}

void Environment::charLiteral(CharacterLiteral *char_lit) {
  mStack.back().setPC(char_lit);
  mStack.back().bindStmt(
      char_lit, ObjectV2(0, 0, static_cast<long>(char_lit->getValue())));
}

void Environment::binop(BinaryOperator *bop) {
  // llvm::dbgs() << "bop: " << bop->getOpcodeStr() << '\n';
  mStack.back().setPC(bop);
  Expr *left = bop->getLHS();
  Expr *right = bop->getRHS();
  auto left_value = mStack.back().getStmtVal(left);
  auto right_value = mStack.back().getStmtVal(right);
  switch (bop->getOpcode()) {
  case clang::BO_Assign: {
    left_value.Assign(right_value);
    mStack.back().bindStmt(left, right_value);
    break;
  }
  case clang::BO_Add: {
    mStack.back().bindStmt(bop, left_value.Add(right_value));
    break;
  }
  case clang::BO_Sub: {
    mStack.back().bindStmt(bop, left_value.Sub(right_value));
    break;
  }
  case clang::BO_Mul: {
    mStack.back().bindStmt(bop, left_value.Mul(right_value));
    break;
  }
  case clang::BO_Div: {
    mStack.back().bindStmt(bop, left_value.Div(right_value));
    break;
  }
  case clang::BO_GE: {
    mStack.back().bindStmt(bop, left_value.Ge(right_value));
    break;
  }
  case clang::BO_GT: {
    mStack.back().bindStmt(bop, left_value.Gt(right_value));
    break;
  }
  case clang::BO_LE: {
    mStack.back().bindStmt(bop, left_value.Le(right_value));
    break;
  }
  case clang::BO_LT: {
    mStack.back().bindStmt(bop, left_value.Lt(right_value));
    break;
  }
  case clang::BO_EQ: {
    mStack.back().bindStmt(bop, left_value.Eq(right_value));
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
  mStack.back().setPC(uop);
  auto value = mStack.back().getStmtVal(uop->getSubExpr());
  switch (uop->getOpcode()) {
  case clang::UO_Plus: {
    mStack.back().bindStmt(uop, value);
    break;
  }
  case clang::UO_Minus: {
    mStack.back().bindStmt(uop, value.Minus());
    break;
  }
  case clang::UO_Deref: {
    mStack.back().bindStmt(uop, value.Deref());
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
  mStack.back().setPC(expr);
  auto ty = expr->getTypeOfArgument();
  // TODO: modify the hacked code
  if (ty->isIntegerType()) {
    mStack.back().bindStmt(expr, ObjectV2(0, 0, 4L));
  } else if (ty->isPointerType()) {
    mStack.back().bindStmt(expr, ObjectV2(0, 0, 8L));
  } else {
    llvm::errs() << "unimplemented unaryOrTypeTrait"
                 << "\n";
    exit(-1);
  }
}

void Environment::decl(DeclStmt *declstmt) {
  mStack.back().setPC(declstmt);
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
          mStack.back().bindDecl(vardecl, ObjectV2(0, 0, 0L));
        } else {
          auto init_value = mStack.back().getStmtVal(init_expr);
          ObjectV2 v = init_value.ToRValue();
          mStack.back().bindDecl(vardecl, v);
        }
      } else if (varDeclType->isPointerType()) {
        unsigned pointerType = getPointerType(varDeclType);
        if (init_expr == nullptr) {
          mStack.back().bindDecl(vardecl, ObjectV2(pointerType, 0, 0L));
        } else {
          auto init_value = mStack.back().getStmtVal(init_expr);
          ObjectV2 v(pointerType, 0, 0L);
          v.Assign(init_value);
          mStack.back().bindDecl(vardecl, v);
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
    // llvm::dbgs() << declref->getDecl()->getDeclName().getAsString() << '\n';
    Decl *decl = declref->getFoundDecl();
    auto val = mStack.back().getDeclValRef(mStack, decl);
    mStack.back().bindStmt(declref, val);
  } else {
    llvm::errs() << "unimplement declref type. name: "
                 << declref->getDecl()->getName()
                 << ", classname: " << declrefType->getTypeClassName() << '\n';
    exit(-1);
  }
}

void Environment::paren(ParenExpr *expr) {
  mStack.back().setPC(expr);
  auto obj = mStack.back().getStmtVal(expr->getSubExpr());
  mStack.back().bindStmt(expr, obj);
}

void Environment::call(CallExpr *callexpr) {
  mStack.back().setPC(callexpr);
  FunctionDecl *callee = callexpr->getDirectCallee();
  if (callee == mInput) {
    long val;
    llvm::errs() << "Please Input an Integer Value : ";
    scanf("%ld", &val);

    mStack.back().bindStmt(callexpr, ObjectV2(0, 0, val));
  } else if (callee == mOutput) {
    Expr *decl = callexpr->getArg(0);
    auto val = mStack.back().getStmtVal(decl);
// #ifndef NDEBUG
//     llvm::errs().enable_colors(true);
//     llvm::errs().changeColor(llvm::raw_ostream::Colors::GREEN, true, false);
// #endif
    llvm::errs() << val.RValue();
// #ifndef NDEBUG
//     llvm::errs().resetColor();
//     llvm::errs().enable_colors(false);
// #endif
  } else if (callee == mMalloc) {
    Expr *decl = callexpr->getArg(0);
    auto val = mStack.back().getStmtVal(decl);
    long n = val.RValue();
    long *ptr = new long[n];
    ObjectV2 arr(1, 0, (long)ptr);
    mHeap.insert(ptr);
    mStack.back().bindStmt(callexpr, arr);
  } else if (callee == mFree) {
    Expr *decl = callexpr->getArg(0);
    auto val = mStack.back().getStmtVal(decl);
    long *ptr = reinterpret_cast<long *>(val.RValue());
    delete[] ptr;
    int res = mHeap.erase(ptr);
    assert(res == 1);
  } else {
    callee = callee->getDefinition();
    assert(callee->getDefinition() == callee);
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
    // llvm::dbgs() << "param: ";
    for (arg = callexpr->arg_begin(), param = callee->param_begin();
         arg != callexpr->arg_end() && param != callee->param_end();
         ++arg, ++param) {
      auto val = mStack.back().getStmtVal(*arg);
      unsigned pointerType = getPointerType((*arg)->getType());
      ObjectV2 v = val.ToRValue();
      stack_frame.bindDecl(*param, v);
      // llvm::dbgs() << "ID=" << (*param)->getID() << ", ";
      // llvm::dbgs() << v.ToString() << ", ";
    }
    mStack.push_back(std::move(stack_frame));
    // llvm::dbgs() << "call begin " << callee->getName() << mStack.size()
    //             << "{\n";
    mVisitor->VisitStmt(callee->getBody());
    mReturned = false;
    // llvm::dbgs() << "call end" << callee->getName() << mStack.size() << "}\n";
    // resume PC
    assert(mRetReg.IsRValue());
    // llvm::dbgs() << "ret: " << mRetReg.ToString() << '\n';
    mStack.pop_back();
    mStack.back().bindStmt(callexpr, mRetReg);
  }
}

void Environment::implicitCast(ImplicitCastExpr *expr) {
  mStack.back().setPC(expr);
  unsigned pointerType = getPointerType(expr->getType());
  auto obj = mStack.back().getStmtVal(expr->getSubExpr());
  obj.CastTo(pointerType);
  mStack.back().bindStmt(expr, obj);
}

void Environment::cast(CastExpr *expr) {
  mStack.back().setPC(expr);
  unsigned pointerType = getPointerType(expr->getType());
  auto obj = mStack.back().getStmtVal(expr->getSubExpr());
  obj.CastTo(pointerType);
  mStack.back().bindStmt(expr, obj);
}

void Environment::arraySubscript(ArraySubscriptExpr *arrSubExpr) {
  mStack.back().setPC(arrSubExpr);
  auto idx = mStack.back().getStmtVal(arrSubExpr->getIdx());
  Expr *baseExpr = arrSubExpr->getBase();
  auto arr = mStack.back().getStmtVal(baseExpr);
  mStack.back().bindStmt(arrSubExpr, arr.Subscript(idx));
}

void Environment::compoundStmtBegin(CompoundStmt *stmt) {
  mStack.back().setPC(stmt);
  // llvm::dbgs() << "\n{\n";
  mStack.emplace_back((mStack.size() - 1));
}

void Environment::compoundStmtEnd() {
  // llvm::dbgs() << "\n}\n";
  mStack.pop_back();
}

void Environment::returnStmt(ReturnStmt *stmt) {
  mStack.back().setPC(stmt);
  // llvm::dbgs() << "return stmt, ";
  Expr *e = stmt->getRetValue();
  if (e != nullptr) {
    mRetReg = mStack.back().getStmtVal(e).ToRValue();
    // llvm::dbgs() << "ret value: " << mRetReg.ToString() << '\n';
  }
  mReturned = true;
}

void Environment::arrayType(VarDecl *vardecl, Expr *init_expr,
                            clang::QualType tp) {
  auto array_tp = dyn_cast<ConstantArrayType>(tp);
  unsigned pointerType = getPointerType(array_tp->getElementType());
  if (init_expr == nullptr) {
    size_t size = array_tp->getSize().getZExtValue();
    long *ptr = new long[size];
    mStack.back().bindDecl(
        vardecl, ObjectV2(pointerType, 0, reinterpret_cast<long>(ptr)));
    mStack.back().mArrs.insert(ptr);
  } else {
    llvm::errs() << "unimplement array initialization.\n";
    exit(-1);
  }
}

void Environment::AddScopeBeforeCompoundStmt() {
  // llvm::dbgs() << "{\n";
  mStack.emplace_back(mStack.size() - 1);
}