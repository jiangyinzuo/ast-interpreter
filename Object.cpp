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
  ToPtr<Assignable>(const_cast<Object *>(this))->Assign(*ToPtr<RValue>(&r));
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

std::unique_ptr<LValueReference> Object::DerefObj() {
  if (Pointer *ptr = dynamic_cast<Pointer *>(this)) {
    return ptr->Deref();
  }
  if (Array *arr = dynamic_cast<Array *>(this)) {
    return arr->Deref();
  }
  if (LValueReference* lv = dynamic_cast<LValueReference*>(this)) {
    return lv->Deref();
  }
  llvm::errs() << "type does not implement deref\n";
  exit(-1);
}

std::unique_ptr<Object> *Object::GetPtrObj() {
  if (Pointer *ptr = dynamic_cast<Pointer *>(this)) {
    return ptr->GetPtr();
  }
  if (Array *arr = dynamic_cast<Array *>(this)) {
    return arr->GetPtr();
  }
  if (LValueReference* lv = dynamic_cast<LValueReference*>(this)) {
    return lv->GetPtr();
  }
  llvm::errs() << "type does not implement get ptr\n";
  exit(-1);
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

void LValueReference::Assign(RValue &r) { (*ref)->AssignObj(r); }

std::unique_ptr<Additive> LValueReference::operator+(Additive &r) const {
  return *(ref->get()) + r;
}

std::unique_ptr<Additive> LValueReference::operator-(Additive &r) const {
  return *(ref->get()) - r;
}

std::unique_ptr<IValue> LValueReference::operator-() const {
  return -*(ref->get());
}

std::unique_ptr<IValue> LValueReference::operator*(IValue &r) const {
  return *(ref->get()) * r;
}

std::unique_ptr<IValue> LValueReference::operator/(IValue &r) const {
  return *(ref->get()) / r;
}

std::unique_ptr<Additive> Array::operator+(Additive &r) const {
  if (Value *v = dynamic_cast<Value *>(&r)) {
    long rvalue = v->GetValue();
    std::unique_ptr<Object> *newBase = this->base + rvalue;
    return std::make_unique<Pointer>(newBase);
  }
  llvm::errs() << "unimplemented Array::opterator+ !\n";
  exit(-1);
}

std::unique_ptr<Additive> Array::operator-(Additive &r) const {
  if (Assignable *v = dynamic_cast<Assignable *>(&r)) {
    long rvalue = v->GetValue();
    std::unique_ptr<Object> *newBase = this->base - rvalue;
    return std::make_unique<Pointer>(newBase);
  }
  llvm::errs() << "unimplemented Array::opterator- !\n";
  exit(-1);
}

std::unique_ptr<LValueReference> Array::operator[](Assignable &v) {
  std::unique_ptr<Object> *newBase = this->base + v.GetValue();
  return std::make_unique<LValueReference>(newBase);
}
std::unique_ptr<Additive> Pointer::operator+(Additive &r) const {
  if (Value *v = dynamic_cast<Value *>(&r)) {
    return std::make_unique<Pointer>(this->base + v->GetValue());
  }
  llvm::errs() << "invalid Pointer::operator+ \n";
  exit(-1);
}

std::unique_ptr<Additive> Pointer::operator-(Additive &r) const {
  if (Value *v = dynamic_cast<Value *>(&r)) {
    return std::make_unique<Pointer>(this->base - v->GetValue());
  }
  llvm::errs() << "invalid Pointer::operator- \n";
  exit(-1);
}

void Pointer::Assign(RValue &r) {
  if (Pointer *ptr = dynamic_cast<Pointer *>(&r)) {
    this->base = ptr->GetPtr();
  } else if (Array *arr = dynamic_cast<Array *>(&r)) {
    this->base = arr->GetPtr();
  } else if (LValueReference *l = dynamic_cast<LValueReference *>(&r)) {
    this->base = l->GetPtr();
  } else {
    llvm::errs() << "type does not implement assign\n";
    exit(-1);
  }
}

long Pointer::GetValue() const { return reinterpret_cast<long>(base); }
