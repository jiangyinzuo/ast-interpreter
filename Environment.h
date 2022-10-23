//==--- tools/clang-check/ClangInterpreter.cpp - Clang Interpreter tool
//--------------===//
//===----------------------------------------------------------------------===//
#pragma once
#include "Object.h"
#include <cassert>
#include <cstdio>
#include <map>
#include <memory>
#include <vector>

namespace clang {
class Decl;
class Stmt;
class FunctionDecl;
class TranslationUnitDecl;
class IntegerLiteral;
class CharacterLiteral;
class BinaryOperator;
class UnaryOperator;
class UnaryExprOrTypeTraitExpr;
class DeclStmt;
class DeclRefExpr;
class CastExpr;
class CallExpr;
class CompoundStmt;
class VarDecl;
class ReturnStmt;
class QualType;
class Expr;
class ParenExpr;
class ArraySubscriptExpr;
class ImplicitCastExpr;
} // namespace clang

class InterpreterVisitor;
using namespace clang;
class StackFrame {
public:
  static constexpr int kNoFather = -1;
  using MappedValue = std::unique_ptr<Object>;

private:
  /// StackFrame maps Variable Declaration to Value
  /// Which are either integer or addresses (also represented using an Integer
  /// value)
  std::map<Decl *, MappedValue> mVars;
  std::map<Stmt *, MappedValue> mExprs;
  std::map<Stmt *, Object *> mDeclRefExprs;

  /// The current stmt
  Stmt *mPC;
  /// return register
  std::unique_ptr<Object> retReg;
  int mFatherID;

public:
  StackFrame(int fatherID)
      : mVars(), mFatherID(fatherID), mExprs(), mPC(), retReg(nullptr) {}

  void bindDecl(Decl *decl, MappedValue val);
  Object *getDeclVal(std::vector<StackFrame> &stack, Decl *declname);

  void bindStmt(Stmt *stmt, Object *val) { mDeclRefExprs[stmt] = val; }
  void bindStmt(Stmt *stmt, MappedValue val) { mExprs[stmt] = std::move(val); }
  
  Object *getStmtVal(Stmt *stmt) const {
    auto res1 = mExprs.find(stmt);
    if (res1 == mExprs.end()) {
      auto res2 = mDeclRefExprs.find(stmt);
      assert(res2 != mDeclRefExprs.end());
      return res2->second;
    }
    return res1->second.get();
  }

  MappedValue MoveStmtVal(Stmt*stmt) {
    auto res1 = mExprs.find(stmt);
    assert(res1 != mExprs.end());
    return std::move(res1->second);
  }

  void setPC(Stmt *stmt) { mPC = stmt; }
  Stmt *getPC() const { return mPC; }
  void setRetReg(MappedValue obj) { retReg = std::move(obj); }
  std::unique_ptr<Object> MoveRetReg() { return std::move(retReg); }
};

class Environment {
  std::vector<StackFrame> mStack;

  FunctionDecl *mFree; /// Declartions to the built-in functions
  FunctionDecl *mMalloc;
  FunctionDecl *mInput;
  FunctionDecl *mOutput;

  FunctionDecl *mEntry;

  std::map<long, std::unique_ptr<Array>> mHeap;

  bool mNextCompoundStmtAddScope;
  InterpreterVisitor *mVisitor;

public:
  /// Get the declartions to the built-in functions
  Environment()
      : mStack(), mFree(NULL), mMalloc(NULL), mInput(NULL), mOutput(NULL),
        mEntry(NULL), mNextCompoundStmtAddScope(true) {}

  /// Initialize the Environment
  void init(TranslationUnitDecl *unit, InterpreterVisitor *mVisitor);

  FunctionDecl *getEntry() { return mEntry; }

  void intLiteral(IntegerLiteral *int_lit);
  void charLiteral(CharacterLiteral *char_lit);

  /// !TODO Support comparison operation
  void binop(BinaryOperator *bop);
  void unary(UnaryOperator *uop);
  void unaryOrTypeTrait(UnaryExprOrTypeTraitExpr *expr);

  void decl(DeclStmt *declstmt);
  void declref(DeclRefExpr *declref);
  void paren(ParenExpr *paren);
  /// !TODO Support Function Call
  void call(CallExpr *callexpr);
  void implicitCast(ImplicitCastExpr *expr);
  void cast(CastExpr *expr);
  void arraySubscript(ArraySubscriptExpr *arrSubExpr);

  void compoundStmtBegin(CompoundStmt *stmt);
  void compoundStmtEnd();

  void returnStmt(ReturnStmt *stmt);

  void arrayType(VarDecl *vardecl, Expr *init_expr, clang::QualType ty);

  long getMainRet() {
    assert(mStack.size() == 2);
    auto ret =  mStack.back().MoveRetReg();
    return ret.get() == nullptr ? 0 : ret->GetValueObj();
  }
};
