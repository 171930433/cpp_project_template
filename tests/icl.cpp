#include <boost/icl/interval_map.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/split_interval_set.hpp>
#include <gtest/gtest.h>
#include <set>
#include <string>

using namespace boost::icl;
using namespace boost::posix_time;
typedef std::set<std::string> GuestSetT;

TEST(icl, base) {

  GuestSetT mary_harry;
  mary_harry.insert("Mary");
  mary_harry.insert("Harry");

  GuestSetT diana_susan;
  diana_susan.insert("Diana");
  diana_susan.insert("Susan");

  GuestSetT peter;
  peter.insert("Peter");

  interval_map<ptime, GuestSetT> party;

  party.add({ interval<ptime>::right_open(time_from_string("2008-05-20 19:30"), time_from_string("2008-05-20 23:00")),
    mary_harry });

  party += std::make_pair(
    interval<ptime>::right_open(time_from_string("2008-05-20 20:10"), time_from_string("2008-05-21 00:00")),
    diana_susan);

  party += make_pair(
    interval<ptime>::right_open(time_from_string("2008-05-20 22:15"), time_from_string("2008-05-21 00:30")), peter);

  for (auto it = party.begin(); it != party.end(); ++it) { GTEST_LOG_(INFO) << it->first << " guset: " << it->second; }
};

TEST(icl, map_join) {

  interval_map<double, double> segments;
  segments += std::make_pair(interval<double>::right_open(1, 3), 1.0);
  segments += std::make_pair(interval<double>::right_open(2, 4), 2.0);
  segments += std::make_pair(interval<double>::right_open(4, 5), 4.0);

  for (auto it = segments.begin(); it != segments.end(); ++it) {
    GTEST_LOG_(INFO) << it->first << " guset: " << it->second;
  }

  // 根据指定区间查找值
  auto its = segments.equal_range(interval<double>::closed(1.5, 1.6));
  for (auto it = its.first; it != its.second; ++it) { GTEST_LOG_(INFO) << it->first << " val = " << it->second; }

  // 根据时刻查找区间
  auto it = segments.find(2.0);
  if (it != segments.end()) { GTEST_LOG_(INFO) << it->first << " " << it->second; }
};

TEST(icl, set_join) {

  interval_set<int> segments;
  segments += interval<int>::right_open(1, 3);
  segments += interval<int>::right_open(2, 4);
  segments += interval<int>::right_open(4, 5);

  for (auto it = segments.begin(); it != segments.end(); ++it) { GTEST_LOG_(INFO) << *it; }

  split_interval_set<int> segments2;
  segments2 += interval<int>::right_open(1, 3);
  segments2 += interval<int>::right_open(2, 4);
  segments2 += interval<int>::right_open(4, 5);

  for (auto it = segments2.begin(); it != segments2.end(); ++it) { GTEST_LOG_(INFO) << *it; }
};