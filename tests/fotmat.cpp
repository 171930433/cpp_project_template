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

namespace ns1 {
enum class film { house_of_cards, american_beauty, se7en = 7 };

auto format_as(film f) {
  return fmt::underlying(f);
}

enum class Color { red, green, blue };

}
template <>
struct fmt::formatter<ns1::Color> : formatter<string_view> {
  // using namespace ns1;
  auto format(ns1::Color color, format_context& ctx) const {
    string_view name = "unknown";
    switch (color) {
      case ns1::Color::red: name = "red"; break;
      case ns1::Color::green: name = "green"; break;
      case ns1::Color::blue: name = "blue"; break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};

TEST(format, enum) {
  ns1::film fm = ns1::film::se7en;
  EXPECT_EQ(fmt::format("{}", fm), "7");

  ns1::Color color = ns1::Color::blue;
  EXPECT_EQ(fmt::format("{}", color), "blue");
}

template <typename _Scalar, int _row, int _col>
struct fmt::formatter<Eigen::Matrix<_Scalar, _row, _col>, std::enable_if_t<(_row != 1 && _col != 1), char>>
  : ostream_formatter {
  auto format(Eigen::Matrix<_Scalar, _row, _col> const& value, format_context& ctx) const {
    return ostream_formatter::format(value.reshaped(1, value.size()), ctx);
  }
};

TEST(format, eigen) {

  Eigen::Vector3i v3i{ 1, 2, 3 };
  EXPECT_EQ(fmt::format("{}", v3i), "[1, 2, 3]");
  EXPECT_EQ(fmt::format("[{}]", fmt::join(v3i, ",")), "[1,2,3]");

  Eigen::Matrix2i mat22;
  mat22 << 1, 2, 3, 4;
  EXPECT_EQ(fmt::format("[{}]", mat22), "[1 3 2 4]");
}
