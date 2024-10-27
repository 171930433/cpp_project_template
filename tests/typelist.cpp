#include <gtest/gtest.h>

template <typename... _Elements>
struct Typelist {};

using SignedIntegralTypes = Typelist<signed char, short, int, long>;

// FrontT
template <typename _List>
struct FrontT;

template <typename _Head, typename... _Tail>
struct FrontT<Typelist<_Head, _Tail...>> {
  using Type = _Head;
};

template <typename _List>
using FrontT_t = typename FrontT<_List>::Type;

// pop frontt
template <typename _List>
struct PopFrontT;

template <typename _Head, typename... _Tail>
struct PopFrontT<Typelist<_Head, _Tail...>> {
  using Type = Typelist<_Tail...>;
};

template <typename _List>
using PopFront_t = typename PopFrontT<_List>::Type;

//  push front T
template <typename _List, typename _NewT>
struct PushFrontT;

template <typename... _Elements, typename _NewT>
struct PushFrontT<Typelist<_Elements...>, _NewT> {
  using Type = Typelist<_NewT, _Elements...>;
};

template <typename _List, typename _NewT>
using PushFront_t = PushFrontT<_List, _NewT>::Type;

// 解剖一个类型列表
TEST(typelist, _24_1) {
  static_assert(std::is_same_v<FrontT_t<SignedIntegralTypes>, signed char>);
  static_assert(std::is_same_v<PopFront_t<SignedIntegralTypes>, Typelist<short, int, long>>);
  static_assert(std::is_same_v<PushFront_t<SignedIntegralTypes, bool>, Typelist<bool, signed char, short, int, long>>);

  EXPECT_TRUE(true);
}

// 列表类型算法
template <typename _List, unsigned _n>
struct NthElement : public NthElement<PopFront_t<_List>, _n - 1> {};

template <typename _List>
struct NthElement<_List, 0> : public FrontT<_List> {};

template <typename _List, unsigned _n>
using NthElement_t = typename NthElement<_List, _n>::Type;

// .1 索引
TEST(typelist, _24_2_1) {
  using TL1 = Typelist<short, int, long>;
  static_assert(std::is_same_v<NthElement_t<TL1, 0>, short>);
  static_assert(std::is_same_v<NthElement_t<TL1, 1>, int>);
  static_assert(std::is_same_v<NthElement_t<TL1, 2>, long>);

  EXPECT_TRUE(1);
}

template <typename _List>
struct LargestType {
private:
  using _First = FrontT_t<_List>;
  using _Rest = typename LargestType<PopFront_t<_List>>::Type;

public:
  using Type = std::conditional_t<sizeof(_First) >= sizeof(_Rest), _First, _Rest>;
};

template <>
struct LargestType<Typelist<>> {
  using Type = char;
};

template <typename _List>
using LargestType_t = typename LargestType<_List>::Type;

// refine with IsEmpty
template <typename _List>
struct IsEmpty : public std::false_type {};

template <>
struct IsEmpty<Typelist<>> : public std::true_type {};

template <typename _List>
constexpr bool IsEmpty_v = IsEmpty<_List>::value;

template <typename _List, bool is_empty = IsEmpty_v<_List>>
struct LargestType2;

template <typename _List>
struct LargestType2<_List, false> {
private:
  using _First = FrontT_t<_List>;
  using _Rest = typename LargestType<PopFront_t<_List>>::Type;

public:
  using Type = std::conditional_t<sizeof(_First) >= sizeof(_Rest), _First, _Rest>;
};

template <typename _List>
struct LargestType2<_List, true> {
  using Type = char;
};

template <typename _List>
using LargestType2_t = typename LargestType2<_List>::Type;

// .2 寻找最佳匹配
TEST(typelist, _24_2_2) {
  using TL1 = Typelist<short, int, long, bool>;
  static_assert(sizeof(bool) == 1);
  static_assert(sizeof(short) == 2);
  static_assert(sizeof(int) == 4);
  static_assert(sizeof(long) == 8);

  static_assert(std::is_same_v<LargestType_t<TL1>, long>);

  static_assert(std::is_same_v<LargestType2_t<TL1>, long>);

  EXPECT_TRUE(1);
}

template <typename _List, typename _NewT>
struct PushBackT;

template <typename... Elements, typename _NewT>
struct PushBackT<Typelist<Elements...>, _NewT> {
  using Type = Typelist<Elements..., _NewT>;
};

template <typename _List, typename _NewT>
using PushBackT_t = typename PushBackT<_List, _NewT>::Type;

// 使用递归的方式
template <typename _list, typename _NewT, bool is_empty = IsEmpty_v<_list>>
struct PushBackRecT2;

template <typename _List, typename _NewT>
struct PushBackRecT2<_List, _NewT, false> {
private:
  using _Head = FrontT_t<_List>;
  using _Tail = PopFront_t<_List>;
  using _NewTail = typename PushBackRecT2<_Tail, _NewT>::Type;

public:
  using Type = PushFront_t<_NewTail, _Head>;
};

template <typename _List, typename _NewT>
struct PushBackRecT2<_List, _NewT, true> {
  using Type = PushFront_t<_List, _NewT>;
};

template <typename _List, typename _NewT>
struct PushBackT2 : public PushBackRecT2<_List, _NewT> {};

template <typename _List, typename _NewT>
using PushBack2_t = typename PushBackT2<_List, _NewT>::Type;

// .3 追加元素到列表类型
TEST(typelist, _24_2_3) {
  using TL = Typelist<>;
  using TL1 = Typelist<bool>;
  using TL2 = Typelist<bool, int>;

  static_assert(std::is_same_v<typename PushBackT2<TL, bool>::Type, Typelist<bool>>);
  static_assert(std::is_same_v<PushBack2_t<TL, bool>, Typelist<bool>>);

  static_assert(std::is_same_v<typename PushBackT2<TL1, int>::Type, Typelist<bool, int>>);
  static_assert(std::is_same_v<PushBack2_t<TL1, int>, Typelist<bool, int>>);

  EXPECT_TRUE(1);
}
