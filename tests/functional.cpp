#include <gtest/gtest.h>

#include <iostream>
#include <vector>

template <typename _R, typename... _Args>
class FunctionalBridge {
public:
  virtual ~FunctionalBridge() = default;
  virtual FunctionalBridge* Clone() const = 0;
  virtual _R Invoke(_Args...) const = 0;
  virtual bool Equal(FunctionalBridge<_R, _Args...> const* fb) = 0;
};

// functor bridge
template <typename _Functor, typename _R, typename... _Args>
class SpecificFunctorBridge : public FunctionalBridge<_R, _Args...> {
  _Functor functor_;

public:
  template <typename _FunctorFwd>
  SpecificFunctorBridge(_FunctorFwd&& functor)
    : functor_(std::forward<_FunctorFwd>(functor)) {}
  //! 返回值类型不同也能重载么?
  SpecificFunctorBridge* Clone() const override { return new SpecificFunctorBridge(functor_); }
  _R Invoke(_Args... args) const override { return functor_(std::forward<_Args>(args)...); }
  bool Equal(FunctionalBridge<_R, _Args...> const* fb) override {
    if (auto spec_fb = dynamic_cast<SpecificFunctorBridge const*>(fb); spec_fb) {
    //   return functor_ == spec_fb->functor_;
      return true;
    }
    return false;
  }
};

// ------------------------------------------------------------------------------------------

template <typename _Signature>
class FunctionalPtr;

// partial specification
template <typename _R, typename... _Args>
class FunctionalPtr<_R(_Args...)> {
private:
  FunctionalBridge<_R, _Args...>* bridge_ = nullptr;

public:
  FunctionalPtr() = default;
  ~FunctionalPtr() { delete bridge_; }
  FunctionalPtr(FunctionalPtr const& other);
  FunctionalPtr(FunctionalPtr& other)
    : FunctionalPtr(static_cast<FunctionalPtr const&>(other)) {}
  FunctionalPtr(FunctionalPtr&& other)
    : bridge_(other.bridge_) {
    other.bridge_ = nullptr;
  }
  // 从任意函数对象构造
  template <typename _F>
  FunctionalPtr(_F&&);

  // =
  FunctionalPtr& operator=(FunctionalPtr const& other) {
    FunctionalPtr tmp(other);
    Swap(tmp, *this);
    return *this;
  }

  FunctionalPtr& operator=(FunctionalPtr&& other) {
    delete bridge_;
    this->bridge_ = other.bridge_;
    other.bridge_ = nullptr;

    return *this;
  }

  template <typename _F>
  FunctionalPtr& operator=(_F&& func) {
    FunctionalPtr tmp(func);
    Swap(func, *this);
    return *this;
  }

  friend void Swap(FunctionalPtr& fp1, FunctionalPtr& fp2) { std::swap(fp1.bridge_, fp2.bridge_); }
  explicit operator bool() const { return bridge_ != nullptr; }
  _R operator()(_Args... args) const { return bridge_->Invoke(std::forward<_Args>(args)...); }

  friend bool operator==(FunctionalPtr const& f1, FunctionalPtr const& f2) {
    if (!f1 || !f2) { return !f1 && !f2; }
    return f1.bridge_->Equal(f2.bridge_);
  }

  friend bool operator!=(FunctionalPtr const& f1, FunctionalPtr const& f2) { return !(f1 == f2); }
};

template <typename _R, typename... _Args>
FunctionalPtr<_R(_Args...)>::FunctionalPtr(FunctionalPtr const& other) {
  if (other.bridge_) { bridge_ = other.bridge_->Clone(); }
}

template <typename _R, typename... _Args>
template <typename _F>
FunctionalPtr<_R(_Args...)>::FunctionalPtr(_F&& func) {
  using Functor = std::decay_t<_F>;
  using Bridge = SpecificFunctorBridge<Functor, _R, _Args...>;
  bridge_ = new Bridge(std::forward<_F>(func)); // 自此, _F的类型不再被FunctionalPtr知道
}

void forUpTo(int n, FunctionalPtr<void(int)> f) {
  for (int i = 0; i < n; ++i) { f(i); }
}

void PrintInt(int i) {
  std::cout << i << ' ';
}

TEST(functional, base) {
  std::vector<int> values;
  std::vector<int> values2{ 0, 1, 2, 3, 4 };
  forUpTo(5, [&values](int i) { values.push_back(i); });

  EXPECT_EQ(values, values2);

  forUpTo(5, PrintInt);
}