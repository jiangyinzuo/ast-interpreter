#pragma once
#include <memory>

namespace clang {
class FunctionDecl;
}

class IValue;
class Pointer;
class Additive;
class LValueReference;

class Object {
public:
  Object(Object &) = delete;
  Object &operator=(Object &) = delete;
  virtual ~Object() = default;

  void AssignObj(Object &r);
  long GetValueObj();
  std::unique_ptr<Additive> operator+(Object &r) const;
  std::unique_ptr<Additive> operator-(Object &r) const;
  std::unique_ptr<IValue> operator-() const;

  std::unique_ptr<IValue> operator*(Object &r) const;
  std::unique_ptr<IValue> operator/(Object &r) const;

	std::unique_ptr<LValueReference> operator[](Object &r) const;

protected:
  Object() = default;
};

class Assignable;

class Additive : public Object {
public:
  virtual ~Additive() = default;
  virtual std::unique_ptr<Additive> operator+(Additive &r) const = 0;
  virtual std::unique_ptr<Additive> operator-(Additive &r) const = 0;
};

class Assignable : public Additive {
public:
  virtual ~Assignable() = default;
  virtual long GetValue() const = 0;
  virtual void Assign(Assignable &r) = 0;
};

class IValue : public Assignable {
public:
  virtual ~IValue() = default;
  virtual std::unique_ptr<IValue> operator-() const = 0;
  virtual std::unique_ptr<IValue> operator*(IValue &r) const = 0;
  virtual std::unique_ptr<IValue> operator/(IValue &r) const = 0;
};

class Value final : public IValue {
public:
  long value;
  explicit Value(long v) : value(v) {}

  long GetValue() const final { return value; }
  void Assign(Assignable &r) final { value = r.GetValue(); }

  std::unique_ptr<Additive> operator+(Additive &r) const final;
  std::unique_ptr<Additive> operator-(Additive &r) const final;
  std::unique_ptr<IValue> operator-() const final;
  std::unique_ptr<IValue> operator*(IValue &r) const final;
  std::unique_ptr<IValue> operator/(IValue &r) const final;
};

static_assert(sizeof(Value) == 16, "");

class LValueReference final : public IValue {
public:
  Object* ref;

	explicit LValueReference(Object* ref) : ref(ref) {}

  long GetValue() const final { return ref->GetValueObj(); }
  void Assign(Assignable &r) final;

  std::unique_ptr<Additive> operator+(Additive &r) const final;
  std::unique_ptr<Additive> operator-(Additive &r) const final;
  std::unique_ptr<IValue> operator-() const final;
  std::unique_ptr<IValue> operator*(IValue &r) const final;
  std::unique_ptr<IValue> operator/(IValue &r) const final;
};

class Function final : public Object {
  const void *p;

public:
  Function(const void *p) : p(p) {}
};

class Array final : public Additive {
public:
  std::unique_ptr<Object> *base;
  size_t size;
  explicit Array(size_t size) : base(new std::unique_ptr<Object>[size]), size(size) {
    for (size_t i = 0; i < size; ++i) {
      base[i] = std::make_unique<Value>(0L);
    }
  }
  ~Array() {
    delete[] base;
    base = nullptr;
  }

  virtual std::unique_ptr<Additive> operator+(Additive &r) const final;
  virtual std::unique_ptr<Additive> operator-(Additive &r) const final;

	std::unique_ptr<LValueReference> operator[](Assignable &v);
};

class Pointer final : public Assignable {
public:
  Object *base;

  explicit Pointer(Object *base) : base(base) {}
	~Pointer() {}

  virtual std::unique_ptr<Additive> operator+(Additive &r) const final;
  virtual std::unique_ptr<Additive> operator-(Additive &r) const final;

  virtual long GetValue() const final {
		return reinterpret_cast<long>(base);
	}

  virtual void Assign(Assignable &r) final;
};
