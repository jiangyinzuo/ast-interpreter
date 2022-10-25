//==--- tools/clang-check/ClangInterpreter.cpp - Clang Interpreter tool
//--------------===//
//===----------------------------------------------------------------------===//
#pragma once
#include "ObjectV2.h"
#include <cassert>
#include <cstdio>
#include <deque>
#include <map>
#include <unordered_set>


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
  static const int kNoFather;

  std::unordered_set<long *> mArrs;

private:
  /// StackFrame maps Variable Declaration to Value
  /// Which are either integer or addresses (also represented using an Integer
  /// value)
  std::map<Decl *, ObjectV2> mVars;
  std::map<Stmt *, ObjectV2> mExprs;

  /// The current stmt
  Stmt *mPC;
  int mFatherID, mID;

public:
  StackFrame(int fatherID)
      : mVars(), mFatherID(fatherID), mExprs(), mPC() {}
  StackFrame(const StackFrame &) = delete;
  StackFrame &operator=(const StackFrame &) = delete;
  StackFrame(StackFrame &&s) {
    std::swap(mVars, s.mVars);
    std::swap(mExprs, s.mExprs);
    std::swap(mPC, s.mPC);
    std::swap(mFatherID, s.mFatherID);
  }
  StackFrame &operator=(StackFrame &&) = delete;

  ~StackFrame();
  void bindDecl(Decl *decl, ObjectV2 val);
  ObjectV2 getDeclValRef(std::deque<StackFrame> &stack, Decl *declname);

  void bindStmt(Stmt *stmt, ObjectV2 val) { mExprs[stmt] = (val); }

  ObjectV2 getStmtVal(Stmt *stmt) const {
    auto res1 = mExprs.find(stmt);
    assert(res1 != mExprs.end());
    return res1->second;
  }

  void setPC(Stmt *stmt) { mPC = stmt; }
  Stmt *getPC() const { return mPC; }
};

class Environment {
  std::deque<StackFrame> mStack;

  FunctionDecl *mFree; /// Declartions to the built-in functions
  FunctionDecl *mMalloc;
  FunctionDecl *mInput;
  FunctionDecl *mOutput;

  FunctionDecl *mEntry;

  std::unordered_set<long *> mHeap;

  InterpreterVisitor *mVisitor;

  ObjectV2 mRetReg;

public:
  bool mReturned;

  /// Get the declartions to the built-in functions
  Environment()
      : mStack(), mFree(NULL), mMalloc(NULL), mInput(NULL), mOutput(NULL),
        mEntry(NULL), mReturned(false) {}

  /// Initialize the Environment
  void init(TranslationUnitDecl *unit, InterpreterVisitor *mVisitor);

  FunctionDecl *getEntry() { return mEntry; }

  void evalStmt(Stmt *stmt);
  void intLiteral(IntegerLiteral *int_lit);
  void charLiteral(CharacterLiteral *char_lit);

  void binop(BinaryOperator *bop);
  void unary(UnaryOperator *uop);
  void unaryOrTypeTrait(UnaryExprOrTypeTraitExpr *expr);

  void decl(DeclStmt *declstmt);
  void declref(DeclRefExpr *declref);
  void paren(ParenExpr *paren);
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
    assert(mRetReg.IsRValue());
    return mRetReg.RValue();
  }

  long getPCValue() const {
    auto *pc = mStack.back().getPC();
    ObjectV2 obj = mStack.back().getStmtVal(pc);
    return obj.RValue();
  }
  void AddScopeBeforeCompoundStmt();
};
