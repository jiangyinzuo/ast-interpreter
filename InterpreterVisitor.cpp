#include "InterpreterVisitor.h"
#include "Environment.h"
void InterpreterVisitor::VisitIntegerLiteral(IntegerLiteral *lit) {
  VisitStmt(lit);
  mEnv->intLiteral(lit);
}
void InterpreterVisitor::VisitCharacterLiteral(CharacterLiteral *lit) {
  VisitStmt(lit);
  mEnv->charLiteral(lit);
}
void InterpreterVisitor::VisitBinaryOperator(BinaryOperator *bop) {
  VisitStmt(bop);
  mEnv->binop(bop);
}
void InterpreterVisitor::VisitUnaryOperator(UnaryOperator *uop) {
  VisitStmt(uop);
  mEnv->unary(uop);
}
void InterpreterVisitor::VisitUnaryExprOrTypeTraitExpr(
    UnaryExprOrTypeTraitExpr *expr) {
  VisitStmt(expr);
  mEnv->unaryOrTypeTrait(expr);
}
void InterpreterVisitor::VisitDeclRefExpr(DeclRefExpr *expr) {
  VisitStmt(expr);
  mEnv->declref(expr);
}

void InterpreterVisitor::VisitImplicitCastExpr(ImplicitCastExpr *expr) {
  VisitStmt(expr);
  mEnv->implicitCast(expr);
}

void InterpreterVisitor::VisitCastExpr(CastExpr *expr) {
  VisitStmt(expr);
  mEnv->cast(expr);
}

void InterpreterVisitor::VisitArraySubscriptExpr(ArraySubscriptExpr *expr) {
  VisitStmt(expr);
  mEnv->arraySubscript(expr);
}

void InterpreterVisitor::VisitParenExpr(ParenExpr *expr) {
  VisitStmt(expr);
  mEnv->paren(expr);
}
void InterpreterVisitor::VisitCallExpr(CallExpr *call) {
  VisitStmt(call);
  mEnv->call(call);
}
void InterpreterVisitor::VisitIfStmt(IfStmt *stmt) {
  VisitStmt(stmt);
  // TODO
}
void InterpreterVisitor::VisitWhileStmt(WhileStmt *stmt) {
  VisitStmt(stmt);
  // TODO
}
void InterpreterVisitor::VisitForStmt(ForStmt *stmt) {
  VisitStmt(stmt);
  // TODO
}
void InterpreterVisitor::VisitDeclStmt(DeclStmt *declstmt) {
  VisitStmt(declstmt);
  mEnv->decl(declstmt);
}
void InterpreterVisitor::VisitCompoundStmt(CompoundStmt *stmt) {
  mEnv->compoundStmtBegin(stmt);
  VisitStmt(stmt);
  mEnv->compoundStmtEnd();
}
void InterpreterVisitor::VisitReturnStmt(ReturnStmt *stmt) {
  VisitStmt(stmt);
  mEnv->returnStmt(stmt);
}
