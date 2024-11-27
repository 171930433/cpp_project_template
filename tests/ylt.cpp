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

template <typename _T, _T... _Names>
struct FieldInfo {};

template <typename _T, std::enable_if_t<iguana::plain_v<_T>>* = nullptr>
inline void GetAllMemberNamesImpl(
  std::string_view name, _T& field, std::vector<std::string>& re, std::string const& pre_fix) {
  re.push_back(pre_fix + std::string(name));
}

template <typename _T, std::enable_if_t<iguana::ylt_refletable_v<_T>>* = nullptr>
inline void GetAllMemberNamesImpl(
  std::string_view name, _T& field, std::vector<std::string>& re, std::string& pre_fix) {

  std::string new_prefix = pre_fix + std::string(name) + ".";

  ylt::reflection::for_each(
    field, [&re, &new_prefix](auto& field, auto name) { GetAllMemberNamesImpl(name, field, re, new_prefix); });
}

template <typename _T, std::enable_if_t<iguana::ylt_refletable_v<_T>, bool> = true>
inline std::vector<std::string> GetAllMemberNames(_T& field) {
  std::vector<std::string> re;

  std::string pre_fix = "";

  ylt::reflection::for_each(
    field, [&re, &pre_fix](auto& field, auto name) { GetAllMemberNamesImpl(name, field, re, pre_fix); });

  return re;
}

TEST(ylt, all_fields) {
  constexpr auto fields = ylt::reflection::get_member_names<Imu>();

  for (auto const& filed : fields) { ELOGD << fmt::format("{}", filed); }

  constexpr auto map2 = ylt::reflection::member_names_map<Imu>;
  for (auto const& [name, no] : map2) { ELOGD << fmt::format("{}={}", std::string_view(name.data(), name.size()), no); }

  Imu i1;

  for_each(i1, [](auto& field, auto name, auto no) { ELOGD << fmt::format("{},{},{}", JsonStr(field), name, no); });

  auto names = GetAllMemberNames(i1);

  for (auto const& filed : names) { ELOGD << fmt::format("{}", filed); }

  Gnss g2;
  names = GetAllMemberNames(g2);

  for (auto const& filed : names) { ELOGD << fmt::format("{}", filed); }

  EXPECT_TRUE(1);
}