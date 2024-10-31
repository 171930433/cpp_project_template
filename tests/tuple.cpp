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

template <typename _T, _T _elem>
struct CTValue {
  static constexpr _T value = _elem;
};

template <typename _T, _T... _elems>
struct CTValuelist {};

template <typename _T, _T _head, _T... _elems>
struct FrontT<CTValuelist<_T, _head, _elems...>> {
  using type = CTValue<_T, _head>;
};

template <typename _T, _T... _elems, unsigned _new>
struct PushFrontT<CTValuelist<_T, _elems...>, CTValue<unsigned, _new>> {
  using type = CTValuelist<_T, _new, _elems...>;
};

template <typename _T, _T _head, _T... _elems>
struct PopFrontT<CTValuelist<_T, _head, _elems...>> {
  using type = CTValuelist<_T, _elems...>;
};

template <typename _Valuelist, unsigned _new>
using PushFront_t2 = typename PushFrontT<_Valuelist, CTValue<unsigned, _new>>::type;

template <typename _T, _T... _elems, unsigned _new>
struct PushBackT<CTValuelist<_T, _elems...>, CTValue<unsigned, _new>> {
  using type = CTValuelist<_T, _elems..., _new>;
};

template <unsigned _n, typename _Result = CTValuelist<unsigned>>
struct MakeIndexListT : MakeIndexListT<_n - 1, PushFront_t2<_Result, _n - 1>> {};

template <typename _Result>
struct MakeIndexListT<0, _Result> {
  using type = _Result;
};

template <unsigned _n>
using MakeIndexList_t = typename MakeIndexListT<_n>::type;

template <typename _T>
struct ReverseT<CTValuelist<_T>> {
  using type = CTValuelist<_T>;
};

template <typename _T, _T _head, _T... _tail>
struct ReverseT<CTValuelist<_T, _head, _tail...>>
  : PushBackT<Reverse_t<CTValuelist<_T, _tail...>>, CTValue<unsigned, _head>> {};

template <typename... _Types, typename _T, _T... _elems>
Reverse_t<Tuple<_Types...>> Reverse2_impl(Tuple<_Types...> const& t, CTValuelist<_T, _elems...>) {
  return { get<_elems>(t)... };
}

template <typename... _Types>
Reverse_t<Tuple<_Types...>> Reverse2(Tuple<_Types...> const& t) {
  return Reverse2_impl(t, Reverse_t<MakeIndexList_t<sizeof...(_Types)>>{});
}

// 25.3.6
template <typename... _Types, unsigned... _indices>
auto Select_impl(Tuple<_Types...> const& t, CTValuelist<unsigned, _indices...>) {
  return MakeTuple(get<_indices>(t)...);
}

template <unsigned... _indices, typename... _Types>
auto Select(Tuple<_Types...> const& t) {
  return Select_impl(t, CTValuelist<unsigned, _indices...>{});
}

template <unsigned _i, unsigned _n, typename _IndexList = CTValuelist<unsigned>>
class ReplicatedIndexListT;

template <unsigned _i, unsigned _n>
using ReplicatedIndexList_t = typename ReplicatedIndexListT<_i, _n>::type;

template <unsigned _i, unsigned... _indices>
class ReplicatedIndexListT<_i, 0, CTValuelist<unsigned, _indices...>> {
public:
  using type = CTValuelist<unsigned, _indices...>;
};

template <unsigned _i, unsigned _n, unsigned... _indices>
class ReplicatedIndexListT<_i, _n, CTValuelist<unsigned, _indices...>>
  : public ReplicatedIndexListT<_i, _n - 1, CTValuelist<unsigned, _indices..., _i>> {};

template <unsigned _i, unsigned _n, typename... _Types>
auto Splat(Tuple<_Types...> const& t) {
  return Select_impl(t, ReplicatedIndexList_t<_i, _n>{});
}

template <typename _Tuple, unsigned _i>
struct NthElementT;

template <typename _Tuple, unsigned _i>
using NthElement_t = typename NthElementT<_Tuple, _i>::type;

template <typename... _Types, unsigned _i>
struct NthElementT<Tuple<_Types...>, _i> : NthElementT<PopFront_t<Tuple<_Types...>>, _i - 1> {};

template <typename... _Types>
struct NthElementT<Tuple<_Types...>, 0> : FrontT<Tuple<_Types...>> {};

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

  // index list
  EXPECT_TRUE((std::is_same_v<MakeIndexList_t<3>, CTValuelist<unsigned, 0, 1, 2>>));
  EXPECT_TRUE((std::is_same_v<Reverse_t<MakeIndexList_t<3>>, CTValuelist<unsigned, 2, 1, 0>>));

  auto t7_true = MakeTuple("xiaoming", 1.0, 1);
  EXPECT_EQ(t7_true, Reverse2(t1));

  // 25.3.6 selset
  // EXPECT_EQ(Select(t1, CTValuelist<unsigned, 0>{}), Tuple<int>{ 1 });
  EXPECT_EQ(Select<0>(t1), Tuple<int>{ 1 });
  EXPECT_EQ((Select<0, 1>(t1)), (Tuple<int, double>{ 1, 1.0 }));
  EXPECT_EQ((Select<0, 2, 1>(t1)), (Tuple<int, char const*, double>{ 1, "xiaoming", 1.0 }));

  EXPECT_EQ((Splat<0, 1>(t1)), Tuple<int>{ 1 });
  EXPECT_EQ((Splat<0, 2>(t1)), (Tuple<int, int>{ 1, 1 }));
  EXPECT_EQ((Splat<0, 3>(t1)), (Tuple<int, int, int>{ 1, 1, 1 }));
  EXPECT_EQ((Splat<0, 4>(t1)), (Tuple<int, int, int, int>{ 1, 1, 1, 1 }));
  EXPECT_EQ((Splat<1, 2>(t1)), (Tuple<double, double>{ 1, 1 }));
  EXPECT_EQ((Splat<2, 2>(t1)), (Tuple<char const*, char const*>{ "xiaoming", "xiaoming" }));

  EXPECT_TRUE((std::is_same_v<NthElement_t<decltype(t1), 0>, int>));
  EXPECT_TRUE((std::is_same_v<NthElement_t<decltype(t1), 1>, double>));
  EXPECT_TRUE((std::is_same_v<NthElement_t<decltype(t1), 2>, char const*>));
}

template <typename _T, _T... _elems>
struct IsEmpty<CTValuelist<_T, _elems...>> : public std::integral_constant<bool, (sizeof...(_elems) == 0)> {};

namespace inner {

// InsertIntoOrdered 将一个类型插入一个有序的typelist
template <typename _List, typename _NewT, template <typename, typename> typename _Compare,
  bool is_empty = IsEmpty_v<_List>>
struct InsertIntoOrdered;

template <typename _List, typename _NewT, template <typename, typename> typename _Compare>
using InsertIntoOrdered_t = typename InsertIntoOrdered<_List, _NewT, _Compare>::type;

template <typename _List, typename _NewT, template <typename, typename> typename _Compare>
struct InsertIntoOrdered<_List, _NewT, _Compare, false>
  : std::conditional_t<_Compare<_NewT, Front_t<_List>>::value, PushFrontT<_List, _NewT>,
      PushFrontT<InsertIntoOrdered_t<PopFront_t<_List>, _NewT, _Compare>, Front_t<_List>>> {};

template <typename _List, typename _NewT, template <typename, typename> typename _Compare>
struct InsertIntoOrdered<_List, _NewT, _Compare, true> : PushFrontT<_List, _NewT> {};

// 插入排序具体实现，_List并不一定有序
template <typename _List, typename _NewT, template <typename, typename> typename _Compare,
  bool is_empty = IsEmpty_v<_List>>
struct InsertSortImpl;

template <typename _List, typename _NewT, template <typename, typename> typename _Compare>
using InsertSort = InsertSortImpl<_List, _NewT, _Compare, IsEmpty_v<_List>>;

template <typename _List, typename _NewT, template <typename, typename> typename _Compare>
using InsertSort_t = typename InsertSort<_List, _NewT, _Compare>::type;

template <typename _List, typename _NewT, template <typename, typename> typename _Compare>
struct InsertSortImpl<_List, _NewT, _Compare, false>
  : InsertIntoOrdered<InsertSort_t<PopFront_t<_List>, Front_t<_List>, _Compare>, _NewT, _Compare> {};

template <typename _List, typename _NewT, template <typename, typename> typename _Compare>
struct InsertSortImpl<_List, _NewT, _Compare, true> : PushFrontT<_List, _NewT> {};

}

template <typename _List, template <typename, typename> typename _Compare,
  typename = std::enable_if_t<!IsEmpty_v<_List>>>
using InsertionSort_t = typename inner::InsertSort<PopFront_t<_List>, Front_t<_List>, _Compare>::type;

template <typename _T1, typename _T2>
struct less_than;

template <typename _T, typename _U>
constexpr bool less_than_v = less_than<_T, _U>::value;

template <typename _T, _T _left, _T _right>
struct less_than<CTValue<_T, _left>, CTValue<_T, _right>> : std::bool_constant<(_left < _right)> {};

template <typename _T, typename _U>
struct less_than : public std::integral_constant<bool, (sizeof(_T) < sizeof(_U))> {};

template <typename _Tuple, template <typename _T, typename _U> typename _F>
class MetafunOfNthElementT {
public:
  template <typename _T, typename _U>
  class Apply;

  template <unsigned _n, unsigned _m>
  class Apply<CTValue<unsigned, _n>, CTValue<unsigned, _m>>
    : public _F<NthElement_t<_Tuple, _n>, NthElement_t<_Tuple, _m>> {};
};

template <template <typename _T, typename _U> typename _Compare, typename... _Types>
auto Sort(Tuple<_Types...> const& t) {
  return Select_impl(t,
    InsertionSort_t<MakeIndexList_t<sizeof...(_Types)>,
      MetafunOfNthElementT<Tuple<_Types...>, _Compare>::template Apply>{});
}

TEST(tuple, 25_3_6) {

  EXPECT_TRUE(
    (std::is_same_v<InsertionSort_t<CTValuelist<unsigned, 3, 2, 1>, less_than>, CTValuelist<unsigned, 1, 2, 3>>));

  EXPECT_TRUE(
    (std::is_same_v<InsertionSort_t<CTValuelist<unsigned, 2, 3, 1>, less_than>, CTValuelist<unsigned, 1, 2, 3>>));

  auto t1 = MakeTuple(int(4), short(2), (long long)(8), char(1));
  auto t2 = Sort<less_than>(t1);
  auto t2_true = Tuple<char, short, int, long long>{ 1, 2, 4, 8 };

  EXPECT_EQ(t2, t2_true);
}

template <typename _Func, typename _Tuple, unsigned... _indices>
auto ApplyImpl(_Func&& func, _Tuple const& t, CTValuelist<unsigned, _indices...>) {
  return func(get<_indices>(t)...);
}

template <typename _Func, typename... _Types>
auto Apply(_Func&& func, Tuple<_Types...> const& t) {
  return ApplyImpl(std::forward<_Func>(func), t, MakeIndexList_t<sizeof...(_Types)>{});
}

double Add(int a, double b) { return a + b; }

// 拓展 apply
TEST(tuple, 25_4) {
  auto t1 = MakeTuple(1, 1.0);
  EXPECT_EQ(Apply(Add, t1), 2);
}

// 优化2， 直接继承，但是会有构造，存储倒叙的违和感

struct CA {
  CA() { GTEST_LOG_(INFO) << "CA"; }
};
struct CB {
  CB() { GTEST_LOG_(INFO) << "CB"; }
};

template <typename...>
class Tuple2;

template <typename _Head, typename... _Tail>
class Tuple2<_Head, _Tail...> : private Tuple2<_Tail...> {
private:
  _Head head_;
};

template <>
class Tuple2<> {};

// TupleElt 优化2， 直接继承， 可以优化空基类 tuple<>的内存占用，但是相同类型会confuse
template <typename... Types>
class Tuple3;

template <typename _T>
class TupleElement {
  _T value_;

public:
  TupleElement() = default;

  template <typename _U>
  TupleElement(_U&& other)
    : value_(std::forward<_U>(other)) {}

  _T& get() { return value_; }
  _T const& get() const { return value_; }
};

template <typename _Head, typename... _Tail>
class Tuple3<_Head, _Tail...>
  : TupleElement<_Head>
  , Tuple3<_Tail...> {};

template <>
class Tuple3<> {};

// 优化3 ，对TupleElement2添加了height的概念，可以存储相同类型
template <unsigned _height, typename _T>
class TupleElement2 {
  _T value_;

public:
  TupleElement2() = default;

  template <typename _U>
  TupleElement2(_U&& other)
    : value_(std::forward<_U>(other)) {}

  _T& get() { return value_; }
  _T const& get() const { return value_; }
};

template <typename... Types>
class Tuple4;

template <typename _Head, typename... _Tail>
class Tuple4<_Head, _Tail...>
  : TupleElement2<sizeof...(_Tail), _Head>
  , Tuple4<_Tail...> {
  using HeadElement = TupleElement2<sizeof...(_Tail), _Head>;

public:
  _Head const& head() const { return static_cast<HeadElement>(this)->get(); }
  _Head& head() { return static_cast<HeadElement>(this)->get(); }
  Tuple4<_Tail...> const& tail() const { return *this; }
  Tuple4<_Tail...>& tail() { return *this; }
};

template <>
class Tuple4<> {};


// 优化
TEST(tuple, 25_5) {
  //
  Tuple2<CA, CB> t0; // ! 先构造CA,再构造CB,有点违背直观感觉

  EXPECT_EQ((sizeof(Tuple<CA, CB>)), 3);
  EXPECT_EQ((sizeof(Tuple2<CA, CB>)), 2); // 继承的方式会有EBCO，所以tuple<> 不再占空间
  EXPECT_EQ((sizeof(Tuple3<CA, CB>)), 2); // 相同的基类会confuse
  EXPECT_EQ((sizeof(Tuple4<CA, CA, CB>)), 3); // 相同的基类不会confuse，但是CA CB都是空类没有EBCO
}