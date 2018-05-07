#include <vector>
#include <string>

#include <gtest/gtest.h>
#include <qi/log.hpp>
#include <qi/application.hpp>
#include <qi/anymodule.hpp>
#include <qi/anyobject.hpp>

#include "cat.hpp"

///
/// \brief TEST
/// The dynamic library used for these tests is compiled from
/// test_packages_anymodule.cpp
///

TEST(TestPackage, TestFreeFunctions)
{
    qi::AnyModule pkg = ::qi::import("qi_test_anymodule");
    ASSERT_TRUE(pkg);

    int r;

    pkg.call<void>("my_void_function", r);
    ASSERT_EQ(r, 44);
    r = pkg.call<int>("my_scl_func");
    ASSERT_EQ(r, 45);

    qi::AnyObject c = pkg.call<qi::AnyObject>("my_cat_returner");

    ASSERT_EQ(c.call<int>("meow"), 15);
}

TEST(TestPackage, FactoryCreatesFunctionalObject)
{
    qi::AnyModule pkg = ::qi::import("qi_test_anymodule");

    ASSERT_TRUE(pkg);

    qi::AnyObject obj = pkg.call<qi::AnyObject>("Cat");
    obj.setProperty("meowVolume", ::qi::AnyValue::from(42));
    int result = obj.call<int>("meow");

    ASSERT_EQ(result, 42);
}
