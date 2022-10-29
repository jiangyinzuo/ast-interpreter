#include "InterpreterVisitor.h"
#include "Environment.h"
void InterpreterVisitor::VisitIntegerLiteral(IntegerLiteral *lit) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(lit);
  mEnv->intLiteral(lit);
}
void InterpreterVisitor::VisitCharacterLiteral(CharacterLiteral *lit) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(lit);
  mEnv->charLiteral(lit);
}
void InterpreterVisitor::VisitBinaryOperator(BinaryOperator *bop) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(bop);
  mEnv->binop(bop);
}
void InterpreterVisitor::VisitUnaryOperator(UnaryOperator *uop) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(uop);
  mEnv->unary(uop);
}
void InterpreterVisitor::VisitUnaryExprOrTypeTraitExpr(
    UnaryExprOrTypeTraitExpr *expr) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(expr);
  mEnv->unaryOrTypeTrait(expr);
}
void InterpreterVisitor::VisitDeclRefExpr(DeclRefExpr *expr) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(expr);
  mEnv->declref(expr);
}

void InterpreterVisitor::VisitImplicitCastExpr(ImplicitCastExpr *expr) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(expr);
  mEnv->implicitCast(expr);
}

void InterpreterVisitor::VisitCastExpr(CastExpr *expr) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(expr);
  mEnv->cast(expr);
}

void InterpreterVisitor::VisitArraySubscriptExpr(ArraySubscriptExpr *expr) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(expr);
  mEnv->arraySubscript(expr);
}

void InterpreterVisitor::VisitParenExpr(ParenExpr *expr) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(expr);
  mEnv->paren(expr);
}

void InterpreterVisitor::VisitCallExpr(CallExpr *call) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(call);
  mEnv->call(call);
}

void InterpreterVisitor::VisitIfStmt(IfStmt *stmt) {
  if (mEnv->mReturned) {
    return;
  }
  mEnv->AddScopeBeforeCompoundStmt();
  {
    Stmt *cond = stmt->getCond();
    // llvm::dbgs() << "if cond: " << cond->getStmtClassName() << '\n';
    Visit(cond);
    if (mEnv->mReturned) {
      mEnv->compoundStmtEnd();
      return;
    }
  }
  long pcValue = mEnv->getPCValue();
  if (pcValue != 0) {
    Stmt *then = stmt->getThen();
    // llvm::dbgs() << "then: " << then->getStmtClassName() << '\n';
    Visit(then);
  } else if (Stmt *e = stmt->getElse()) {
    // llvm::dbgs() << "else: " << e->getStmtClassName() << '\n';
    Visit(e);
  }
  mEnv->compoundStmtEnd();
}

void InterpreterVisitor::VisitWhileStmt(WhileStmt *stmt) {
  if (mEnv->mReturned) {
    return;
  }
  mEnv->AddScopeBeforeCompoundStmt();
  for (;;) {
    {
      auto cond = stmt->getCond();
      // llvm::dbgs() << "while cond: " << cond->getStmtClassName() << '\n';
      Visit(cond);
      if (mEnv->mReturned) {
        mEnv->compoundStmtEnd();
        return;
      }
      long pcValue = mEnv->getPCValue();
      if (pcValue == 0) {
        break;
      }
    }

    if (Stmt *body = stmt->getBody()) {
      // llvm::dbgs() << "while body: " << body->getStmtClassName() << '\n';
      Visit(body);
      if (mEnv->mReturned) {
        mEnv->compoundStmtEnd();
        return;
      }
    }
  }
  mEnv->compoundStmtEnd();
}

void InterpreterVisitor::VisitForStmt(ForStmt *stmt) {
  if (mEnv->mReturned) {
    return;
  }
  mEnv->AddScopeBeforeCompoundStmt();
  if (Stmt *s = stmt->getInit()) {
    // llvm::dbgs() << "for init: " << s->getStmtClassName() << '\n';
    Visit(s);
    if (mEnv->mReturned) {
      mEnv->compoundStmtEnd();
      return;
    }
  }
  while (!mEnv->mReturned) {
    if (Stmt *condS = (stmt->getCond())) {
      // llvm::dbgs() << "for cond: " << condS->getStmtClassName() << '\n';
      Visit(condS);
      if (mEnv->mReturned) {
        break;
      }
      long pcValue = mEnv->getPCValue();
      if (pcValue == 0) {
        break;
      }
    }
    if (Stmt *body = stmt->getBody()) {
      // llvm::dbgs() << "for body: " << body->getStmtClassName() << '\n';
      Visit(body);
      if (mEnv->mReturned) {
        break;
      }
    }
    if (Stmt *inc = stmt->getInc()) {
      // llvm::dbgs() << "for inc: " << inc->getStmtClassName() << '\n';
      Visit(inc);
      if (mEnv->mReturned) {
        break;
      }
    }
  }
  mEnv->compoundStmtEnd();
}

void InterpreterVisitor::VisitDeclStmt(DeclStmt *declstmt) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(declstmt);
  mEnv->decl(declstmt);
}
void InterpreterVisitor::VisitCompoundStmt(CompoundStmt *stmt) {
  if (mEnv->mReturned) {
    return;
  }
  mEnv->compoundStmtBegin(stmt);
  VisitStmt(stmt);
  mEnv->compoundStmtEnd();
}
void InterpreterVisitor::VisitReturnStmt(ReturnStmt *stmt) {
  if (mEnv->mReturned) {
    return;
  }
  VisitStmt(stmt);
  mEnv->returnStmt(stmt);
}
