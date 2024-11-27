#include <gtest/gtest.h>

#include "message/sensors.h"
#include <frozen/map.h>
#include <ylt/easylog.hpp>
#include <ylt/reflection/member_names.hpp>
#include <ylt/reflection/member_value.hpp>
#include <ylt/struct_json/json_writer.h>

using namespace ylt::reflection;

template <typename _T, std::enable_if_t<iguana::ylt_refletable_v<_T>>* = nullptr>
inline std::string JsonStr(_T const& elem) {
  std::string s;
  struct_json::to_json(elem, s);
  return s;
}

template <typename _T, std::enable_if_t<iguana::plain_v<_T>>* = nullptr>
inline std::string JsonStr(_T const& elem) {
  std::stringstream ss;
  ss << elem;
  return ss.str();
}

template <typename _T, std::enable_if_t<iguana::plain_v<_T>>* = nullptr>
inline void GetAllMemberNamesImpl(std::string_view name, _T& field) {
  std::string const filed_name(name);
  char const* str = (std::stringstream() << field).str().c_str();
}

template <typename _T, std::enable_if_t<iguana::ylt_refletable_v<_T>>* = nullptr>
inline void GetAllMemberNamesImpl(std::string_view name, _T& field) {
  std::string const filed_name(name);
  ylt::reflection::for_each(field, [](auto& field, auto name) { GetAllMemberNamesImpl(name, field); });
}

template <typename _T, std::enable_if_t<iguana::ylt_refletable_v<_T>, bool> = true>
inline void GetAllMemberNames(_T& field) {
  std::string const filed_name(ylt::reflection::type_string<_T>());

  ylt::reflection::for_each(field, [](auto& field, auto name) { GetAllMemberNamesImpl(name, field); });
}

TEST(ylt, all_fields) {
  constexpr auto fields = ylt::reflection::get_member_names<Imu>();

  for (auto const& filed : fields) { ELOGD << fmt::format("{}", filed); }

  constexpr auto map2 = ylt::reflection::member_names_map<Imu>;
  for (auto const& [name, no] : map2) { ELOGD << fmt::format("{}={}", std::string_view(name.data(), name.size()), no); }

  Imu i1;

  for_each(i1, [](auto& field, auto name, auto no) { ELOGD << fmt::format("{},{},{}", JsonStr(field), name, no); });

  EXPECT_TRUE(1);
}