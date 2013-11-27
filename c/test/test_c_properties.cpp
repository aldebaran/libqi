/*
** Author(s):
**  - Guillaume OREAL <goreal@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#include <qic/future.h>

#include <gtest/gtest.h>
#include <qic/object.h>
#include <qic/application.h>
#include <qic/value.h>

TEST(TestProperties, simple)
{
  qi_future_t *fut_set, *fut_get;

  qi_object_builder_t* ob = qi_object_builder_create();
  qi_object_builder_advertise_property(ob, "myproperty", "s");
  qi_object_t* obj = qi_object_builder_get_object(ob);
  qi_object_builder_destroy(ob);

  qi_value_t *val = qi_value_create("s");
  qi_value_set_string(val, "oasis is good");
  fut_set = qi_object_set_property(obj, "myproperty", val);
  qi_value_destroy(val);
  qi_future_wait(fut_set, QI_FUTURETIMEOUT_INFINITE);
//  std::cout << qi_future_get_error(fut_set) << std::endl;
  ASSERT_TRUE(!qi_future_has_error(fut_set, QI_FUTURETIMEOUT_NONE));

  qi_future_destroy(fut_set);

  fut_get = qi_object_get_property(obj, "myproperty");
  qi_future_wait(fut_get, QI_FUTURETIMEOUT_INFINITE);

  ASSERT_TRUE(qi_future_has_value(fut_get, QI_FUTURETIMEOUT_NONE));

  qi_value_t *vret = qi_future_get_value(fut_get);
  std::cout << "value kind:" << (int)qi_value_get_kind(vret) << std::endl;

  const char * mystring = qi_value_get_string(qi_future_get_value(fut_get));

  std::cout <<"value:"<< mystring;
  free((void*) mystring);
  qi_future_destroy(fut_get);
}

TEST(TestProperties, set_args_null)
{
  qi_future_t *fut_1, *fut_2, *fut_3;

  qi_object_builder_t* ob = qi_object_builder_create();
  qi_object_builder_advertise_property(ob, "myproperty", "(s)");
  qi_object_t* obj = qi_object_builder_get_object(ob);

  qi_value_t *val = qi_value_create("s");
  qi_value_set_string(val ,"oasis is good");


  fut_1 = qi_object_set_property(NULL, "myproperty", val);
  fut_2 = qi_object_set_property(obj, NULL, val);
  fut_3 = qi_object_set_property(obj, "myproperty", NULL);


  ASSERT_TRUE(qi_future_has_error(fut_1, QI_FUTURETIMEOUT_INFINITE));
  ASSERT_TRUE(qi_future_has_error(fut_2, QI_FUTURETIMEOUT_INFINITE));
  ASSERT_TRUE(qi_future_has_error(fut_3, QI_FUTURETIMEOUT_INFINITE));
  qi_future_destroy(fut_1);
  qi_future_destroy(fut_2);
  qi_future_destroy(fut_3);
}

TEST(TestProperties, get_args_null)
{
  qi_future_t *fut_set;

  qi_object_builder_t* ob = qi_object_builder_create();
  qi_object_builder_advertise_property(ob, "myproperty", "(s)");
  qi_object_t* obj = qi_object_builder_get_object(ob);

  qi_value_t *val = qi_value_create("s");
  qi_value_set_string(val ,"oasis is good");
  fut_set = qi_object_set_property(obj, "myproperty", val);
  qi_future_wait(fut_set, QI_FUTURETIMEOUT_INFINITE);

  ASSERT_TRUE(qi_future_has_error(qi_object_get_property(NULL, "myproperty"), QI_FUTURETIMEOUT_INFINITE));
  ASSERT_TRUE(qi_future_has_error(qi_object_get_property(obj, NULL), QI_FUTURETIMEOUT_INFINITE));
}


int main(int argc, char **argv) {
  qi_application_t*     app;
  app = qi_application_create(&argc, argv);
  ::testing::InitGoogleTest(&argc, argv);

  RUN_ALL_TESTS();
  qi_application_destroy(app);

}
