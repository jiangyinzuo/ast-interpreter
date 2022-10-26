#include "ObjectV2.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>

void ObjectV2::Assign(const ObjectV2 &obj) {
  if (pointerType != obj.pointerType) {
    llvm::errs() << "different type: " << pointerType << " " << obj.pointerType
                 << "\n";
    exit(-1);
  }
  long *addr = &rawValue;
  for (int i = 0; i < derefCount; ++i) {
    addr = (long *)(*addr);
  }
  *addr = obj.RValue();
}

ObjectV2 ObjectV2::Add(const ObjectV2 &obj) const {
  if (pointerType > 0 && obj.pointerType > 0) {
    llvm::errs() << "invalid add\n";
    exit(-1);
  } else if (obj.pointerType > 0 && pointerType == 0) {
    return obj.Add(*this);
  } else if (pointerType > 0 && obj.pointerType == 0) {
    return ObjectV2(pointerType, 0, RValue() + 8 * obj.RValue());
  } else {
    return ObjectV2(pointerType, 0, RValue() + obj.RValue());
  }
}

ObjectV2 ObjectV2::Sub(const ObjectV2 &obj) const {
  if (pointerType > 0 && obj.pointerType > 0) {
    llvm::errs() << "invalid sub\n";
    exit(-1);
  } else if (obj.pointerType > 0 && pointerType == 0) {
    return obj.Add(*this);
  } else if (pointerType > 0 && obj.pointerType == 0) {
    return ObjectV2(pointerType, 0, RValue() - 8 * obj.RValue());
  } else {
    return ObjectV2(pointerType, 0, RValue() - obj.RValue());
  }
}

ObjectV2 ObjectV2::Mul(const ObjectV2 &obj) const {
  if (pointerType > 0 || obj.pointerType > 0) {
    llvm::errs() << "invalid mul\n";
    exit(-1);
  }
  return ObjectV2(0, 0, RValue() * obj.RValue());
}

ObjectV2 ObjectV2::Div(const ObjectV2 &obj) const {
  if (pointerType > 0 || obj.pointerType > 0) {
    llvm::errs() << "invalid div\n";
    exit(-1);
  }
  return ObjectV2(0, 0, RValue() / obj.RValue());
}

ObjectV2 ObjectV2::Minus() const {
  if (pointerType > 0) {
    llvm::errs() << "invalid minus\n";
    exit(-1);
  }
  return ObjectV2(0, 0, -RValue());
}

ObjectV2 ObjectV2::Gt(const ObjectV2 &obj) const {
  if (pointerType > 0 || obj.pointerType > 0) {
    llvm::errs() << "invalid gt\n";
    exit(-1);
  }
  return ObjectV2(0, 0, RValue() > obj.RValue());
}
ObjectV2 ObjectV2::Ge(const ObjectV2 &obj) const {
  if (pointerType > 0 || obj.pointerType > 0) {
    llvm::errs() << "invalid ge\n";
    exit(-1);
  }
  return ObjectV2(0, 0, RValue() >= obj.RValue());
}
ObjectV2 ObjectV2::Lt(const ObjectV2 &obj) const {

  if (pointerType > 0 || obj.pointerType > 0) {
    llvm::errs() << "invalid lt\n";
    exit(-1);
  }
  return ObjectV2(0, 0, RValue() < obj.RValue());
}
ObjectV2 ObjectV2::Le(const ObjectV2 &obj) const {
  if (pointerType > 0 || obj.pointerType > 0) {
    llvm::errs() << "invalid le\n";
    exit(-1);
  }
  return ObjectV2(0, 0, RValue() <= obj.RValue());
}

ObjectV2 ObjectV2::Eq(const ObjectV2 &obj) const {
  if (pointerType > 0 || obj.pointerType > 0) {
    llvm::errs() << "invalid eq\n";
    exit(-1);
  }
  return ObjectV2(0, 0, RValue() == obj.RValue());
}
ObjectV2 ObjectV2::Deref() const {
  if (pointerType == 0) {
    llvm::errs() << "invalid deref\n";
    exit(-1);
  }
  return ObjectV2(pointerType - 1, derefCount + 1, rawValue);
}

ObjectV2 ObjectV2::Subscript(const ObjectV2 &obj) const {
  ObjectV2 ptr = this->Add(obj);
  return ptr.Deref();
}
