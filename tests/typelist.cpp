#include <gtest/gtest.h>

template <typename... _Elements>
struct Typelist {};

template <typename _T>
struct Wrapper {
  using Type = _T;
};
using EmptyListWrapper = Wrapper<Typelist<>>;

using SignedIntegralTypes = Typelist<signed char, short, int, long>;

// FrontT
template <typename _List>
struct FrontT;

template <typename _Head, typename... _Tail>
struct FrontT<Typelist<_Head, _Tail...>> {
  using Type = _Head;
};

template <typename _List>
using Front_t = typename FrontT<_List>::Type;

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
  static_assert(std::is_same_v<Front_t<SignedIntegralTypes>, signed char>);
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
  using _First = Front_t<_List>;
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
  using _First = Front_t<_List>;
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
  using _Head = Front_t<_List>;
  using _Tail = PopFront_t<_List>;
  using _NewTail = typename PushBackRecT2<_Tail, _NewT>::Type;

public:
  using Type = PushFront_t<_NewTail, _Head>;
};

template <typename _List, typename _NewT>
struct PushBackRecT2<_List, _NewT, true> {
  using Type = PushFront_t<_List, _NewT>;
};

// 先写具体的实现PushBackRecT2, 再写PushBackT2做具体的递归,再写PushBack2_t简化类型萃取
template <typename _List, typename _NewT>
struct PushBackT2 : public PushBackRecT2<_List, _NewT> {};

template <typename _List, typename _NewT>
using PushBack2_t = typename PushBackT2<_List, _NewT>::Type;

// 这样结构会差一些PushBackRecT2
template <typename _List, typename _NewT>
using PushBack3_t = typename PushBackRecT2<_List, _NewT>::Type;

// .3 追加元素到列表类型
TEST(typelist, _24_2_3) {
  using TL = Typelist<>;
  using TL1 = Typelist<bool>;
  using TL2 = Typelist<bool, int>;

  static_assert(std::is_same_v<typename PushBackT2<TL, bool>::Type, Typelist<bool>>);
  static_assert(std::is_same_v<PushBack2_t<TL, bool>, Typelist<bool>>);

  static_assert(std::is_same_v<typename PushBackT2<TL1, int>::Type, Typelist<bool, int>>);
  static_assert(std::is_same_v<PushBack2_t<TL1, int>, Typelist<bool, int>>);

  static_assert(std::is_same_v<PushBack3_t<TL1, int>, Typelist<bool, int>>);
  static_assert(std::is_same_v<PushBack3_t<TL, bool>, Typelist<bool>>);

  EXPECT_TRUE(1);
}

template <typename _List, bool is_empty = IsEmpty_v<_List>>
struct Reverse;

template <typename _List>
using Reverse_t = typename Reverse<_List>::Type;

template <typename _List>
struct Reverse<_List, true> {
  using Type = _List;
};

template <typename _List>
struct Reverse<_List, false> {
private:
  using _Head = Front_t<_List>;
  using _Tail = PopFront_t<_List>;

public:
  using Type = PushBack2_t<Reverse_t<_Tail>, _Head>;
};

// use impl
template <typename _List, bool is_empty>
struct ReverseImpl;

template <typename _List>
using Reverse2_t = typename ReverseImpl<_List, IsEmpty_v<_List>>::Type;

template <typename _List>
struct ReverseImpl<_List, true> : EmptyListWrapper {};

template <typename _List>
struct ReverseImpl<_List, false> : PushBackT2<Reverse2_t<PopFront_t<_List>>, Front_t<_List>> {};

template <typename _List>
using PopBack_t = Reverse2_t<PopFront_t<Reverse2_t<_List>>>;

// .4 反转类型列表
TEST(typelist, _24_2_4) {
  using TL = Typelist<>;
  using TL1 = Typelist<bool>;
  using TL2 = Typelist<bool, int>;

  static_assert(std::is_same_v<Reverse_t<TL>, Typelist<>>);
  static_assert(std::is_same_v<Reverse_t<TL1>, Typelist<bool>>);
  static_assert(std::is_same_v<Reverse_t<TL2>, Typelist<int, bool>>);

  static_assert(std::is_same_v<Reverse2_t<TL>, Typelist<>>);
  static_assert(std::is_same_v<Reverse2_t<TL1>, Typelist<bool>>);

  static_assert(std::is_same_v<PopBack_t<TL1>, Typelist<>>);
  static_assert(std::is_same_v<PopBack_t<TL2>, Typelist<bool>>);

  EXPECT_TRUE(1);
}

template <typename _List, template <typename> typename _MetaFun, bool is_empty>
struct TransformImpl;

template <typename _List, template <typename> typename _MetaFun>
using TransformT = TransformImpl<_List, _MetaFun, IsEmpty_v<_List>>;

template <typename _List, template <typename> typename _MetaFun>
using Transform_t = typename TransformT<_List, _MetaFun>::Type;

template <typename _List, template <typename> typename _MetaFun>
struct TransformImpl<_List, _MetaFun, true> : EmptyListWrapper {};

template <typename _List, template <typename> typename _MetaFun>
struct TransformImpl<_List, _MetaFun, false>
  : PushFrontT<Transform_t<PopFront_t<_List>, _MetaFun>, typename _MetaFun<Front_t<_List>>::Type> {};

template <typename _T>
using AddConst = Wrapper<std::add_const_t<_T>>;

// .5 转化类型列表
TEST(typelist, _24_2_5) {
  using TL = Typelist<>;
  using TL1 = Typelist<bool>;
  using TL2 = Typelist<bool, int>;

  static_assert(std::is_same_v<Transform_t<TL, AddConst>, Typelist<>>);
  static_assert(std::is_same_v<Transform_t<TL1, AddConst>, Typelist<bool const>>);
  static_assert(std::is_same_v<Transform_t<TL2, AddConst>, Typelist<bool const, int const>>);

  EXPECT_TRUE(1);
}

template <typename _List, template <typename, typename> typename _Func, typename _InitType, bool is_empty>
struct AccumulateImpl;

template <typename _List, template <typename, typename> typename _Func, typename _InitType>
using AccumulateT = AccumulateImpl<_List, _Func, _InitType, IsEmpty_v<_List>>;

template <typename _List, template <typename, typename> typename _Func, typename _InitType>
using Accumulate_t = typename AccumulateT<_List, _Func, _InitType>::Type;

template <typename _List, template <typename, typename> typename _Func, typename _InitType>
struct AccumulateImpl<_List, _Func, _InitType, true> : Wrapper<_InitType> {};

template <typename _List, template <typename, typename> typename _Func, typename _InitType>
struct AccumulateImpl<_List, _Func, _InitType, false>
  : AccumulateT<PopFront_t<_List>, _Func, typename _Func<_InitType, Front_t<_List>>::Type> {};

// _Func is largest
template <typename _T1, typename _T2>
using LargerType = Wrapper<std::conditional_t<(sizeof(_T1) > sizeof(_T2)), _T1, _T2>>;

template <typename _List>
using Reverse3_t = Accumulate_t<_List, PushFrontT, Typelist<>>;

template <typename _List, bool is_empty = IsEmpty_v<_List>>
struct LargestType3T : AccumulateT<_List, LargerType, bool> {};

template <typename _List>
struct LargestType3T<_List, true> {};

template <typename _List>
using LargestType3_t = typename LargestType3T<_List>::Type;

// .6 累加类型列表
TEST(typelist, _24_2_6) {
  using TL0 = Typelist<>;
  using TL1 = Typelist<bool>;
  using TL2 = Typelist<bool, short>;
  using TL3 = Typelist<bool, short, int>;

  static_assert(std::is_same_v<Accumulate_t<TL2, LargerType, int>, int>);
  static_assert(std::is_same_v<Accumulate_t<TL2, LargerType, bool>, short>);
  static_assert(std::is_same_v<Accumulate_t<TL3, LargerType, bool>, int>);
  static_assert(std::is_same_v<Accumulate_t<TL3, LargerType, int>, int>);
  static_assert(std::is_same_v<Accumulate_t<TL3, LargerType, long>, long>);

  static_assert(std::is_same_v<Reverse3_t<TL1>, Typelist<bool>>);
  static_assert(std::is_same_v<Reverse3_t<TL2>, Typelist<short, bool>>);
  static_assert(std::is_same_v<Reverse3_t<TL3>, Typelist<int, short, bool>>);

  // static_assert(std::is_same_v<LargestType3_t<TL0>, bool>);
  static_assert(std::is_same_v<LargestType3_t<TL1>, bool>);
  static_assert(std::is_same_v<LargestType3_t<TL2>, short>);
  static_assert(std::is_same_v<LargestType3_t<TL3>, int>);

  EXPECT_TRUE(1);
}

template <typename _List, template <typename, typename> typename _Compare, bool is_empty>
struct InsertionSortImpl;

namespace inner {

// InsertIntoOrdered 将一个类型插入一个有序的typelist
template <typename _List, typename _NewT, template <typename, typename> typename _Compare,
  bool is_empty = IsEmpty_v<_List>>
struct InsertIntoOrdered;

template <typename _List, typename _NewT, template <typename, typename> typename _Compare>
using InsertIntoOrdered_t = typename InsertIntoOrdered<_List, _NewT, _Compare>::Type;

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
using InsertSort_t = typename InsertSort<_List, _NewT, _Compare>::Type;

template <typename _List, typename _NewT, template <typename, typename> typename _Compare>
struct InsertSortImpl<_List, _NewT, _Compare, false>
  : InsertIntoOrdered<InsertSort_t<PopFront_t<_List>, Front_t<_List>, _Compare>, _NewT, _Compare> {};

template <typename _List, typename _NewT, template <typename, typename> typename _Compare>
struct InsertSortImpl<_List, _NewT, _Compare, true> : PushFrontT<_List, _NewT> {};

}

template <typename _List, template <typename, typename> typename _Compare>
using InsertionSortT = InsertionSortImpl<_List, _Compare, IsEmpty_v<_List>>;

template <typename _List, template <typename, typename> typename _Compare>
using InsertionSort_t = typename InsertionSortT<_List, _Compare>::Type;

template <typename _List, template <typename, typename> typename _Compare>
struct InsertionSortImpl<_List, _Compare, true> : Wrapper<_List> {};

template <typename _List, template <typename, typename> typename _Compare>
struct InsertionSortImpl<_List, _Compare, false> : inner::InsertSort<PopFront_t<_List>, Front_t<_List>, _Compare> {};

template <typename _T1, typename _T2>
struct less_than : std::conditional_t<(sizeof(_T1) < sizeof(_T2)), std::true_type, std::false_type> {};

void InsertSort(std::vector<int>& arr, int n) {
  if (n <= 1) return;

  InsertSort(arr, n - 1);

  int last = arr[n - 1];
  int j = n - 2;

  while (j >= 0 && arr[j] > last) {
    arr[j + 1] = arr[j];
    --j;
  }
  arr[j + 1] = last;
}

// .7 插入排序
TEST(typelist, _24_2_7) {
  using TL0 = Typelist<>;
  using TL1 = Typelist<bool>;
  using TL2 = Typelist<bool, short>;
  using TL3 = Typelist<bool, short, int>;
  using TL11 = Typelist<bool>;
  using TL22 = Typelist<short, bool>;
  using TL33 = Typelist<int, short, bool>;

  static_assert(std::is_same_v<PushFront_t<Typelist<int>, short>, Typelist<short, int>>);
  static_assert(
    std::is_same_v<inner::InsertIntoOrdered_t<Typelist<bool, int>, short, less_than>, Typelist<bool, short, int>>);

  using T0 = typename inner::InsertIntoOrdered<Typelist<int>, short, less_than>::Type;
  static_assert(std::is_same_v<T0, Typelist<short, int>>);

  static_assert(std::is_same_v<InsertionSort_t<TL0, less_than>, Typelist<>>);
  static_assert(std::is_same_v<InsertionSort_t<TL1, less_than>, Typelist<bool>>);
  static_assert(std::is_same_v<InsertionSort_t<TL2, less_than>, Typelist<bool, short>>);
  static_assert(std::is_same_v<InsertionSort_t<TL3, less_than>, Typelist<bool, short, int>>);

  static_assert(std::is_same_v<InsertionSort_t<TL11, less_than>, TL1>);
  static_assert(std::is_same_v<InsertionSort_t<TL22, less_than>, TL2>);
  EXPECT_TRUE((std::is_same_v<InsertionSort_t<TL22, less_than>, TL2>));
  static_assert(std::is_same_v<InsertionSort_t<TL33, less_than>, TL3>);

  std::vector<int> arr{ 4, 2, 1 };
  InsertSort(arr, arr.size());

  EXPECT_EQ(arr, (std::vector<int>{ 1, 2, 4 }));

  EXPECT_TRUE(1);
}
