#pragma once

#include "clang/AST/EvaluatedExprVisitor.h"
using namespace clang;
class Environment;
class InterpreterVisitor : public EvaluatedExprVisitor<InterpreterVisitor> {
public:
  explicit InterpreterVisitor(const ASTContext &context, Environment *env)
      : EvaluatedExprVisitor(context), mEnv(env) {}
  ~InterpreterVisitor() {}

  void VisitIntegerLiteral(IntegerLiteral *lit);
  void VisitCharacterLiteral(CharacterLiteral *lit);
  void VisitBinaryOperator(BinaryOperator *bop);
  void VisitUnaryOperator(UnaryOperator *uop);
  void VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *expr);
  void VisitDeclRefExpr(DeclRefExpr *expr);
  void VisitArraySubscriptExpr(ArraySubscriptExpr *expr);
  void VisitParenExpr(ParenExpr *expr);
  void VisitCallExpr(CallExpr *call);
  void VisitImplicitCastExpr(ImplicitCastExpr *expr);
  void VisitCastExpr(CastExpr *expr);
  void VisitIfStmt(IfStmt *stmt);
  void VisitWhileStmt(WhileStmt *stmt);
  void VisitForStmt(ForStmt *stmt);
  void VisitDeclStmt(DeclStmt *declstmt);
  void VisitCompoundStmt(CompoundStmt *stmt);
  void VisitReturnStmt(ReturnStmt *stmt);

private:
  Environment *mEnv;
};
