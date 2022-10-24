#pragma once
class ObjectV2 {
private:
  // Type
  unsigned pointerType;

  // Value
  // prvalue: derefCount = 0
  int derefCount;
  long rawValue;

public:
	ObjectV2() : pointerType(0), derefCount(0), rawValue(0) {}
  explicit ObjectV2(unsigned pointerType, int derefCount, long rawValue)
      : derefCount(derefCount), pointerType(pointerType), rawValue(rawValue) {}

  void Assign(const ObjectV2 &obj);

  long RValue() const {
    long res = rawValue;
    for (int i = 0; i < derefCount; ++i) {
      res = *(long *)res;
    }
    return res;
  }

	void CastTo(unsigned pointerType) {
		this->pointerType = pointerType;
	}

  // Return RValue
  ObjectV2 Add(const ObjectV2 &obj) const;
  ObjectV2 Sub(const ObjectV2 &obj) const;
  ObjectV2 Mul(const ObjectV2 &obj) const;
  ObjectV2 Div(const ObjectV2 &obj) const;
	ObjectV2 Minus() const;
  ObjectV2 Gt(const ObjectV2 &obj) const;
  ObjectV2 Ge(const ObjectV2 &obj) const;
  ObjectV2 Lt(const ObjectV2 &obj) const;
  ObjectV2 Le(const ObjectV2 &obj) const;
  ObjectV2 ToRValue() const {
    return ObjectV2(pointerType, 0, RValue());
  }
  
  // Return LValue
  ObjectV2 Deref() const;
  ObjectV2 Subscript(const ObjectV2 &obj) const;
	ObjectV2 LValueRef() const {
		return ObjectV2{pointerType, derefCount + 1, (long)&rawValue};
	}
};