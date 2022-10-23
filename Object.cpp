#include "Object.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
#include <memory>

template <class Ptr> Ptr *ToPtr(Object *obj) {
  if (Ptr *result = dynamic_cast<Ptr *>(obj)) {
    return result;
  }
  llvm::errs() << "failed to cast\n";
  exit(-1);
}

void Object::AssignObj(Object &r) {
  ToPtr<Assignable>(const_cast<Object *>(this))->Assign(*ToPtr<Assignable>(&r));
}

long Object::GetValueObj() {
  return ToPtr<Assignable>(const_cast<Object *>(this))->GetValue();
}

std::unique_ptr<Additive> Object::operator+(Object &r) const {
  return *ToPtr<Additive>(const_cast<Object *>(this)) + *ToPtr<Additive>(&r);
}

std::unique_ptr<Additive> Object::operator-(Object &r) const {
  return *ToPtr<Additive>(const_cast<Object *>(this)) - *ToPtr<Additive>(&r);
}
std::unique_ptr<IValue> Object::operator-() const {
  return -*ToPtr<IValue>(const_cast<Object *>(this));
}

std::unique_ptr<IValue> Object::operator*(Object &r) const {
  return *ToPtr<IValue>(const_cast<Object *>(this)) * *ToPtr<IValue>(&r);
}

std::unique_ptr<IValue> Object::operator/(Object &r) const {
  return *ToPtr<IValue>(const_cast<Object *>(this)) / *ToPtr<IValue>(&r);
}

std::unique_ptr<LValueReference> Object::operator[](Object &r) const {
  return (*ToPtr<Array>(const_cast<Object *>(this)))[*ToPtr<Assignable>(&r)];
}

std::unique_ptr<Additive> Value::operator+(Additive &r) const {
  if (Value *v = dynamic_cast<Value *>(&r)) {
    auto r = std::make_unique<Value>(this->GetValue() + v->GetValue());
    return r;
  }
  llvm::errs() << "invalid operator + \n";
  exit(-1);
}

std::unique_ptr<Additive> Value::operator-(Additive &r) const {
  if (Value *v = dynamic_cast<Value *>(&r)) {
    auto r = std::make_unique<Value>(this->GetValue() - v->GetValue());
    return r;
  }
  llvm::errs() << "invalid operator -() \n";
  exit(-1);
}

std::unique_ptr<IValue> Value::operator-() const {
  auto r = std::make_unique<Value>(-this->value);
  return r;
}

std::unique_ptr<IValue> Value::operator*(IValue &r) const {
  if (Value *v = dynamic_cast<Value *>(&r)) {
    auto r = std::make_unique<Value>(this->GetValue() * v->GetValue());
    return r;
  }
  llvm::errs() << "invalid operator * \n";
  exit(-1);
}

std::unique_ptr<IValue> Value::operator/(IValue &r) const {
  if (Value *v = dynamic_cast<Value *>(&r)) {
    if (v->value == 0) {
      llvm::errs() << "div zero!\n";
      exit(-1);
    }
    auto r = std::make_unique<Value>(this->GetValue() / v->GetValue());
    return r;
  }
  llvm::errs() << "invalid operator / \n";
  exit(-1);
}

void LValueReference::Assign(Assignable &r) {
  ToPtr<Assignable>(ref)->Assign(r);
}

std::unique_ptr<Additive> LValueReference::operator+(Additive &r) const {
  return *ToPtr<Additive>(ref) + r;
}

std::unique_ptr<Additive> LValueReference::operator-(Additive &r) const {
  return *ToPtr<Additive>(ref) - r;
}

std::unique_ptr<IValue> LValueReference::operator-() const {
  return -*ToPtr<IValue>(ref);
}

std::unique_ptr<IValue> LValueReference::operator*(IValue &r) const {
  return *ToPtr<IValue>(ref) * r;
}

std::unique_ptr<IValue> LValueReference::operator/(IValue &r) const {
  return *ToPtr<IValue>(ref) / r;
}

std::unique_ptr<Additive> Array::operator+(Additive &r) const {
  if (Value *v = dynamic_cast<Value *>(&r)) {
    long rvalue = v->GetValue();
    Object *newBase = this->base[rvalue].get();
    return std::make_unique<Pointer>(newBase);
  }
  llvm::errs() << "unimplemented Array::opterator+ !\n";
  exit(-1);
}

std::unique_ptr<Additive> Array::operator-(Additive &r) const {
  if (Assignable *v = dynamic_cast<Assignable *>(&r)) {
    long rvalue = v->GetValue();
    Object *newBase = this->base[-rvalue].get();
    return std::make_unique<Pointer>(newBase);
  }
  llvm::errs() << "unimplemented Array::opterator+ !\n";
  exit(-1);
}

std::unique_ptr<LValueReference> Array::operator[](Assignable &v) {
  Object *newBase = this->base[v.GetValue()].get();
  return std::make_unique<LValueReference>(newBase);
}
std::unique_ptr<Additive> Pointer::operator+(Additive &r) const {
  if (Value *v = dynamic_cast<Value *>(&r)) {
    return std::make_unique<Pointer>(reinterpret_cast<Object*>(this->GetValue() + v->GetValue()));
  }
  llvm::errs() << "invalid Pointer::operator+ \n";
  exit(-1);
}

std::unique_ptr<Additive> Pointer::operator-(Additive &r) const {
  if (Value *v = dynamic_cast<Value *>(&r)) {
    return std::make_unique<Pointer>(reinterpret_cast<Object*>(this->GetValue() - v->GetValue()));
  }
  llvm::errs() << "invalid Pointer::operator+ \n";
  exit(-1);
}

void Pointer::Assign(Assignable &r) {
  base = reinterpret_cast<Object *>(r.GetValue());
}
