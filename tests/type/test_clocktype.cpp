/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/clock.hpp>
#include <qi/anyvalue.hpp>
#include <limits>

TEST(TestClockType, ClockType)
{
  qi::NanoSeconds  ns(std::numeric_limits<qi::uint64_t>::max());
  qi::MicroSeconds us(1);
  qi::MilliSeconds ms(2);
  qi::Seconds      s(3);
  qi::Minutes      mm(4);
  qi::Hours        h(5);

  qi::uint64_t uns = qi::AnyValue::from(ns).to<qi::uint64_t>();
  qi::uint64_t uus = qi::AnyValue::from(us).to<qi::uint64_t>();
  qi::uint64_t ums = qi::AnyValue::from(ms).to<qi::uint64_t>();
  qi::uint64_t uls = qi::AnyValue::from(s).to<qi::uint64_t>();
  qi::uint64_t umm = qi::AnyValue::from(mm).to<qi::uint64_t>();
  qi::uint64_t uh  = qi::AnyValue::from(h).to<qi::uint64_t>();

  EXPECT_EQ(uns, std::numeric_limits<qi::uint64_t>::max());
  EXPECT_EQ(uus, 1000ULL);
  EXPECT_EQ(ums, 2000000ULL);
  EXPECT_EQ(uls, 3000000000ULL);
  EXPECT_EQ(umm, 4ULL * 60ULL * 1000000000ULL);
  EXPECT_EQ(uh,  5ULL * 60ULL * 60ULL * 1000000000ULL);


  qi::NanoSeconds  ns2;
  qi::MicroSeconds us2;
  qi::MilliSeconds ms2;
  qi::Seconds      s2;
  qi::Minutes      mm2;
  qi::Hours        h2;

  ns2 = qi::AnyValue::from(uns).to<qi::NanoSeconds>();
  us2 = qi::AnyValue::from(uus).to<qi::MicroSeconds>();
  ms2 = qi::AnyValue::from(ums).to<qi::MilliSeconds>();
  s2  = qi::AnyValue::from(uls).to<qi::Seconds>();
  mm2 = qi::AnyValue::from(umm).to<qi::Minutes>();
  h2  = qi::AnyValue::from(uh).to<qi::Hours>();

  EXPECT_EQ(ns2, ns);
  EXPECT_EQ(us2, us);
  EXPECT_EQ(ms2, ms);
  EXPECT_EQ(s2, s);
  EXPECT_EQ(mm2, mm);
  EXPECT_EQ(h2, h);

  EXPECT_THROW(qi::AnyValue::from(ns).to<qi::uint8_t>(), std::runtime_error);
}



TEST(TestClockType, TimePoint)
{
  qi::SystemClockTimePoint wctp(qi::NanoSeconds(42));

  qi::uint64_t tp = qi::AnyValue::from(wctp).to<qi::uint64_t>();

  EXPECT_EQ(42u, tp);

  qi::SystemClockTimePoint wctp2 = qi::AnyValue::from(42).to<qi::SystemClockTimePoint>();
  EXPECT_EQ(wctp, wctp2);
}
