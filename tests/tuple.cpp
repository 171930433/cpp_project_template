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

// IsEmpty
template <typename... _Types>
struct IsEmpty : std::false_type {};

template <>
struct IsEmpty<Tuple<>> : std::true_type {};

template <typename... _Types>
constexpr bool IsEmpty_v = IsEmpty<_Types...>::value;

// FrontT
template <typename _Tuple>
struct FrontT;

template <typename _Head, typename... _Tail>
struct FrontT<Tuple<_Head, _Tail...>> {
  using type = _Head;
};

template <typename _Tuple>
using Front_t = typename FrontT<_Tuple>::type;

// pop FrontT
template <typename _Tuple>
struct PopFrontT;

template <typename _Head, typename... _Tail>
struct PopFrontT<Tuple<_Head, _Tail...>> {
  using type = Tuple<_Tail...>;
};

template <typename _Tuple>
using PopFront_t = typename PopFrontT<_Tuple>::type;

// push front t
template <typename _Tuple, typename _New>
struct PushFrontT;

template <typename... _Types, typename _New>
struct PushFrontT<Tuple<_Types...>, _New> {
  using type = Tuple<_New, _Types...>;
};

template <typename _Tuple, typename _New>
using PushFront_t = typename PushFrontT<_Tuple, _New>::type;

// push back t
template <typename _Tuple, typename _New>
struct PushBackT;

template <typename... _Types, typename _New>
struct PushBackT<Tuple<_Types...>, _New> {
  using type = Tuple<_Types..., _New>;
};

template <typename _Tuple, typename _New>
using PushBack_t = typename PushBackT<_Tuple, _New>::type;

// 25.3.2 添加或删除元素
template <typename... _Types, typename _New>
auto PushFront(Tuple<_Types...>& t, _New const& elem) {
  return PushFront_t<Tuple<_Types...>, _New>{ elem, t };
}

template <typename _New>
Tuple<_New> PushBack(Tuple<> const& t, _New const& elem) {
  return { elem };
}

template <typename _Head, typename... _Tail, typename _New>
Tuple<_Head, _Tail..., _New> PushBack(Tuple<_Head, _Tail...> const& t, _New const& elem) {
  return { t.head(), PushBack(t.tail(), elem) };
}

template <typename... _Types>
PopFront_t<Tuple<_Types...>> PopFront(Tuple<_Types...> const& t) {
  return { t.tail() };
}

template <typename _Tuple>
struct ReverseT;

template <typename _Tuple>
using Reverse_t = typename ReverseT<_Tuple>::type;

template <>
struct ReverseT<Tuple<>> {
  using type = Tuple<>;
};

template <typename _Head, typename... _Tail>
struct ReverseT<Tuple<_Head, _Tail...>> : PushBackT<Reverse_t<Tuple<_Tail...>>, _Head> {};

Tuple<> Reverse(Tuple<> const& t) { return t; }

template <typename... _Types>
Reverse_t<Tuple<_Types...>> Reverse(Tuple<_Types...> const& t) {
  return PushBack(Reverse(t.tail()), t.head());
}

// pop back t
template <typename _Tuple>
struct PopBackT;

template <typename _Tuple>
using PopBack_t = typename PopBackT<_Tuple>::type;

template <typename... _Types>
struct PopBackT<Tuple<_Types...>> : ReverseT<PopFront_t<Reverse_t<Tuple<_Types...>>>> {};

template <typename... _Types>
PopBack_t<Tuple<_Types...>> PopBack(Tuple<_Types...> const& t) {
  return Reverse(PopFront(Reverse(t)));
}

// 25.3 算法
TEST(tuple, 25_3) {
  Tuple<> t0;
  auto t1 = MakeTuple(1, 1.0, "xiaoming");

  EXPECT_EQ(IsEmpty_v<decltype(t0)>, true);
  EXPECT_EQ(IsEmpty_v<decltype(t1)>, false);

  EXPECT_TRUE((std::is_same_v<Front_t<decltype(t1)>, int>));
  EXPECT_TRUE((std::is_same_v<PopFront_t<decltype(t1)>, Tuple<double, char const*>>));
  EXPECT_TRUE((std::is_same_v<PushFront_t<decltype(t1), bool>, Tuple<bool, int, double, char const*>>));
  EXPECT_TRUE((std::is_same_v<PushBack_t<decltype(t1), bool>, Tuple<int, double, char const*, bool>>));

  // 25.3.2
  auto t2_true = MakeTuple(true, 1, 1.0, "xiaoming");
  auto t3_true = MakeTuple(1, 1.0, "xiaoming", true);
  auto t2 = PushFront(t1, true);
  auto t3 = PushBack(t1, true);

  EXPECT_EQ(t2, t2_true);
  EXPECT_EQ(t3, t3_true);

  auto t4_true = MakeTuple(1.0, "xiaoming");
  auto t4 = PopFront(t1);
  EXPECT_EQ(t4, t4_true);

  // reverse
  EXPECT_TRUE((std::is_same_v<Reverse_t<Tuple<>>, Tuple<>>));
  EXPECT_TRUE((std::is_same_v<Reverse_t<Tuple<int>>, Tuple<int>>));
  EXPECT_TRUE((std::is_same_v<Reverse_t<Tuple<int, double>>, Tuple<double, int>>));
  EXPECT_TRUE((std::is_same_v<Reverse_t<Tuple<int, double, long>>, Tuple<long, double, int>>));
  EXPECT_TRUE((std::is_same_v<Reverse_t<decltype(t1)>, Tuple<char const*, double, int>>));
  auto t5_true = MakeTuple("xiaoming", 1.0, 1);
  EXPECT_EQ(t5_true, Reverse(t1));

  // pop backT
  EXPECT_TRUE((std::is_same_v<PopBack_t<Tuple<int>>, Tuple<>>));
  EXPECT_TRUE((std::is_same_v<PopBack_t<Tuple<int, double>>, Tuple<int>>));
  EXPECT_TRUE((std::is_same_v<PopBack_t<Tuple<int, double, long>>, Tuple<int, double>>));
  auto t6_true = MakeTuple(1, 1.0);
  EXPECT_EQ(t6_true, PopBack(t1));
}