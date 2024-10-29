#include <gtest/gtest.h>

template <typename... _Types>
class Tuple;

template <typename _Head, typename... _Tail>
class Tuple<_Head, _Tail...> {
private:
  _Head head_;
  Tuple<_Tail...> tail_;

public:
  Tuple() = default;
  Tuple(_Head const& head, Tuple<_Tail...> const& tail)
    : head_(head)
    , tail_(tail) {}

  Tuple(_Head const& head, _Tail const&... tail);

  template <typename _VHead, typename... _VTail, typename = std::enable_if_t<(sizeof...(_Tail) == sizeof...(_VTail))>>
  Tuple(_VHead&& head, _VTail&&... tail);

  template <typename _VHead, typename... _VTail, typename = std::enable_if_t<(sizeof...(_Tail) == sizeof...(_VTail))>>
  Tuple(Tuple<_VHead, _VTail...> const& other);

  _Head const& head() const { return head_; }
  _Head& head() { return head_; }
  Tuple<_Tail...> const& tail() const { return tail_; }
  Tuple<_Tail...>& tail() { return tail_; }
};

template <>
class Tuple<> {};

template <unsigned int _N>
struct TupleGet {
  template <typename _Head, typename... _Tail>
  static auto apply(Tuple<_Head, _Tail...> const& t) {
    return TupleGet<_N - 1>::apply(t.tail());
  }
};

template <>
struct TupleGet<0> {
  template <typename _Head, typename... _Tail>
  static auto apply(Tuple<_Head, _Tail...> const& t) {
    return t.head();
  }
};

template <unsigned int _N, typename... _Types>
auto get(Tuple<_Types...> const& t) {
  return TupleGet<_N>::apply(t);
}

// 1. 存储
TEST(tuple, 25_1_1) {
  Tuple<int, double, std::string> t0{ 1,
    Tuple<double, std::string>{ 1.0, Tuple<std::string>{ "xiaoming", Tuple<>{} } } };

  EXPECT_EQ(1, get<0>(t0));
  EXPECT_EQ(1.0, get<1>(t0));
  EXPECT_EQ("xiaoming", get<2>(t0));
}

template <typename _Head, typename... _Tail>
Tuple<_Head, _Tail...>::Tuple(_Head const& head, _Tail const&... tail)
  : head_(head)
  , tail_(tail...) {}

template <typename _Head, typename... _Tail>
template <typename _VHead, typename... _VTail, typename>
Tuple<_Head, _Tail...>::Tuple(_VHead&& head, _VTail&&... tail)
  : head_(std::forward<_VHead>(head))
  , tail_(std::forward<_VTail>(tail)...) {}

template <typename _Head, typename... _Tail>
template <typename _VHead, typename... _VTail, typename>
Tuple<_Head, _Tail...>::Tuple(Tuple<_VHead, _VTail...> const& other)
  : head_(other.head())
  , tail_(other.tail()) {}

template <typename... Types>
auto MakeTuple(Types&&... elems) {
  return Tuple<std::decay_t<Types>...>(std::forward<Types>(elems)...);
}

// 2. 构造
TEST(tuple, 25_1_2) {
  Tuple<int, double, std::string> t0{ 1, 1.0, "xiaoming" };
  EXPECT_EQ(1, get<0>(t0));
  EXPECT_EQ(1.0, get<1>(t0));
  EXPECT_EQ("xiaoming", get<2>(t0));

  Tuple<long int, long double, std::string> t3(1, 1.0, "xiaoming");

  EXPECT_EQ(1, get<0>(t3));
  EXPECT_EQ(1.0, get<1>(t3));
  EXPECT_EQ("xiaoming", get<2>(t3));

  Tuple<long int, long double, std::string> t4(t0);
  EXPECT_EQ(1, get<0>(t4));
  EXPECT_EQ(1.0, get<1>(t4));
  EXPECT_EQ("xiaoming", get<2>(t4));

  auto t5 = MakeTuple(1, 1.0, "xiaoming");
  EXPECT_EQ(1, get<0>(t5));
  EXPECT_EQ(1.0, get<1>(t5));
  EXPECT_EQ("xiaoming", get<2>(t5));
}

// 25.2.1

bool operator==(Tuple<> const& left, Tuple<> const& right) { return true; }

template <typename _Head1, typename... _Tail1, typename _Head2, typename... _Tail2>
bool operator==(Tuple<_Head1, _Tail1...> const& left, Tuple<_Head2, _Tail2...> const& right) {
  return false;
}

template <typename _Head, typename... _Tail>
bool operator==(Tuple<_Head, _Tail...> const& left, Tuple<_Head, _Tail...> const& right) {
  return (left.head() == right.head()) && (left.tail() == right.tail());
}

// 25.2.2 输出
void PrintTuple(std::ostream& os, Tuple<> const&, bool is_first = true) { os << (is_first ? '(' : ')'); }

template <typename _Head, typename... _Tail>
void PrintTuple(std::ostream& os, Tuple<_Head, _Tail...> const& t, bool is_first = true) {
  os << (is_first ? "(" : ", ");
  os << t.head();
  PrintTuple(os, t.tail(), false);
}

template <typename... _Types>
std::ostream& operator<<(std::ostream& os, Tuple<_Types...> const& t) {
  PrintTuple(os, t);
  return os;
}

// 25.2 基础操作
TEST(tuple, 25_2) {
  auto t1 = MakeTuple(1, 1.0, "xiaoming");
  std::ostringstream oss;
  oss << t1;

  EXPECT_EQ(oss.str(), "(1, 1, xiaoming)");

  auto t2(t1);
  auto t3 = MakeTuple(2, 1.0, "xiaoming");
  auto t4 = MakeTuple("100", 1.0, "xiaoming");

  EXPECT_EQ(t1, t2);
  EXPECT_FALSE(t1 == t3);
  EXPECT_FALSE(t1 == t4);
}