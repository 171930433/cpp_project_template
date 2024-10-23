#include <deque>
#include <eigen3/Eigen/Dense>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <gtest/gtest.h>

TEST(format, base) {

  std::vector ints{ 1, 2, 3 };
  EXPECT_EQ(fmt::format("{}", ints), "[1, 2, 3]");
  EXPECT_EQ(fmt::format("[{}]", fmt::join(ints, ",")), "[1,2,3]");
}

enum class film { house_of_cards, american_beauty, se7en = 7 };
enum class Color { red, green, blue };
auto format_as(film f) { return fmt::underlying(f); }

template <>
struct fmt::formatter<Color> : formatter<string_view> {
  // using namespace ns1;
  auto format(Color color, format_context& ctx) const {
    string_view name = "unknown";
    switch (color) {
      case Color::red: name = "red"; break;
      case Color::green: name = "green"; break;
      case Color::blue: name = "blue"; break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};

TEST(format, enum) {
  film fm = film::se7en;
  EXPECT_EQ(fmt::format("{}", fm), "7");

  Color color = Color::blue;
  EXPECT_EQ(fmt::format("{}", color), "blue");
}

struct AS {
  virtual ~AS() = default;
  virtual std::string name() const { return "A"; }
};

struct BS : AS {
  virtual std::string name() const { return "B"; }
};

template <typename _T>
struct fmt::formatter<_T, std::enable_if_t<std::is_base_of_v<AS, _T>, char>> : formatter<std::string> {
  auto format(AS const& elem, format_context& ctx) const { return formatter<std::string>::format(elem.name(), ctx); }
};

template <typename _T>
struct fmt::formatter<_T,
  std::enable_if_t<std::is_base_of_v<Eigen::DenseBase<_T>, _T>
      && (Eigen::internal::traits<_T>::RowsAtCompileTime != 1 && Eigen::internal::traits<_T>::ColsAtCompileTime != 1),
    char>> : ostream_formatter {

  enum class EigenFmt { None, Clean };

  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin();
    while (it != ctx.end() && *it != '}') {
      if (*it == 'c') { fmt_ = EigenFmt::Clean; }
      ++it;
    }
    return it;
  }

  auto format(_T const& elem, format_context& ctx) const {
    Eigen::IOFormat io{};

    switch (fmt_) {
      case EigenFmt::Clean: io = Eigen::IOFormat(4, 0, ", ", "\n", "[", "]"); break;
    }
    return ostream_formatter::format(elem.reshaped(1, elem.size()).format(io), ctx);
  }
  EigenFmt fmt_ = EigenFmt::None;
};

TEST(format, type) {
  AS a;
  BS b;
  EXPECT_EQ(fmt::format("{}", a), "A");
  EXPECT_EQ(fmt::format("{}", b), "B");
}

TEST(format, eigen) {

  Eigen::Vector3i v3i{ 1, 2, 3 };
  EXPECT_EQ(fmt::format("{}", v3i), "[1, 2, 3]");
  EXPECT_EQ(fmt::format("[{}]", fmt::join(v3i, ",")), "[1,2,3]");

  Eigen::Matrix2i mat22;
  mat22 << 1, 2, 3, 4;
  EXPECT_EQ(fmt::format("[{}]", mat22), "[1 3 2 4]");
  EXPECT_EQ(fmt::format("{:c}", mat22), "[1, 3, 2, 4]");
}
