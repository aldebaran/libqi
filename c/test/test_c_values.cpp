/*
** Author(s):
**  - Guillaume OREAL <gorealn@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#include <qic/future.h>
#include <gtest/gtest.h>
#include <qic/object.h>
#include <qic/application.h>
#include <qic/value.h>
#include <qic/session.h>

void print(const char *signature, qi_value_t *message, qi_value_t *answer, void *data)
{
  printf("coucou\n");
}

void spawn_obj(const char *signature, qi_value_t *message, qi_value_t *answer, void *data)
{
  qi_object_builder_t* ob = qi_object_builder_create();
  qi_object_builder_advertise_method(ob, "print::v(s)", &print, 0);
  qi_object_t* obj = qi_object_builder_get_object(ob);

  qi_value_t* obj_value = qi_value_create("m");
  qi_value_set_object(obj_value, obj);

  qi_object_builder_destroy(ob);
  //qi_object_destroy(obj);

  printf("#################SPAWNING~~~~~~~~~\n");
  answer = obj_value;
}

TEST(testValue, testkindandsignature)
{
  qi_value_t * myint = qi_value_create("i");
  ASSERT_TRUE(qi_value_get_kind(myint) == QI_VALUE_KIND_INT);
  ASSERT_TRUE(*qi_value_get_signature(myint, 0) == 'i');

  qi_value_t * myfloat = qi_value_create("f");
  ASSERT_TRUE(qi_value_get_kind(myfloat) == QI_VALUE_KIND_FLOAT);
  ASSERT_TRUE(*qi_value_get_signature(myfloat, 0) == 'f');

  qi_value_t * mystring = qi_value_create("s");
  ASSERT_TRUE(qi_value_get_kind(mystring) == QI_VALUE_KIND_STRING);
  ASSERT_TRUE(*qi_value_get_signature(mystring, 0) == 's');

  qi_value_t * mylist = qi_value_create("[s]");
  ASSERT_TRUE(qi_value_get_kind(mylist) == QI_VALUE_KIND_LIST);
  ASSERT_TRUE(strcmp(qi_value_get_signature(mylist, 0), "[s]") == 0);

  qi_value_t * mymap = qi_value_create("{is}");
  ASSERT_TRUE(qi_value_get_kind(mymap) == QI_VALUE_KIND_MAP);
  ASSERT_TRUE(strcmp(qi_value_get_signature(mymap, 0), "{is}") == 0);

  qi_value_t * myobject = qi_value_create("o");
  ASSERT_TRUE(qi_value_get_kind(myobject) == QI_VALUE_KIND_DYNAMIC);
  ASSERT_TRUE(strcmp(qi_value_get_signature(myobject, 0), "o") == 0);

  //qi_value_t * mypointer = qi_value_create("");

  qi_value_t * mytuple = qi_value_create("(iii)");
  ASSERT_TRUE(qi_value_get_kind(mytuple) == QI_VALUE_KIND_TUPLE);
  ASSERT_TRUE(strcmp(qi_value_get_signature(mytuple, 0), "(iii)") == 0);

  qi_value_t * mydynamic = qi_value_create("m");
  ASSERT_TRUE(qi_value_get_kind(mydynamic) == QI_VALUE_KIND_DYNAMIC);
  ASSERT_TRUE(strcmp(qi_value_get_signature(mydynamic, 0), "m") == 0);

  qi_value_destroy(myint);
  qi_value_destroy(myfloat);
  qi_value_destroy(mystring);
  qi_value_destroy(mylist);
  qi_value_destroy(mymap);
  qi_value_destroy(mytuple);
  qi_value_destroy(mydynamic);
  qi_value_destroy(myobject);

}

TEST(testValue, basictest) //Todo test destroy,
{
  qi_value_t *myint = qi_value_create("i");
  qi_value_set_int64(myint, 42);

  qi_value_t *mycopy = qi_value_copy(myint);//make a copy

  long long result; //check the copy
  qi_value_get_int64(mycopy, &result);
  ASSERT_TRUE(result == 42);
  ASSERT_TRUE(qi_value_get_kind(mycopy) == QI_VALUE_KIND_INT);
  ASSERT_TRUE(*qi_value_get_signature(mycopy, 0) == 'i');

  qi_value_set_int64(mycopy, 43);//set one value to 43 then swap
  qi_value_swap(mycopy, myint);

  qi_value_get_int64(mycopy, &result);//check if the swap is ok.
  ASSERT_TRUE(result == 42);
  qi_value_get_int64(myint, &result);
  ASSERT_TRUE(result == 43);

  qi_value_reset(mycopy, "s");//reset the value.
  qi_value_get_int64(mycopy, &result);

  ASSERT_FALSE(result == 42);// check if the value is reset.
  ASSERT_TRUE(qi_value_get_kind(mycopy) == QI_VALUE_KIND_STRING);
  ASSERT_TRUE(*qi_value_get_signature(mycopy, 0) == 's');

  qi_value_destroy(mycopy);
  qi_value_destroy(myint);
}
//Int
TEST(testValue, testint)
{
  //int64
  qi_value_t *myint = qi_value_create("i");

  long long result;
  qi_value_set_int64(myint, 42);
  qi_value_get_int64(myint, &result);

  ASSERT_TRUE(result == 42);

  result = 0;
  result = qi_value_get_int64_default(myint, 0);
  ASSERT_TRUE(result == 42);
  qi_value_destroy(myint);

  //uint64
  qi_value_t *myuint = qi_value_create("i");
  unsigned long long uresult;

  qi_value_set_uint64(myuint, 51);
  qi_value_get_uint64(myuint, &uresult);
  ASSERT_TRUE(uresult == 51);

  uresult = 0;
  uresult = qi_value_get_uint64_default(myuint, 0);
  ASSERT_TRUE(uresult == 51);
  qi_value_destroy(myuint);
}

//FLOAT
TEST(testValue, testfloat)
{
  qi_value_t *myfloat = qi_value_create("f");
  qi_value_set_float(myfloat, 4.2);

  float float_return;
  qi_value_get_float(myfloat, &float_return);
  ASSERT_TRUE((float_return - 4.2) < 0.000001);

  float_return = 0.0;
  float_return = qi_value_get_float_default(myfloat, 0.0);
  ASSERT_TRUE((float_return - 4.2) < 0.000001);
  qi_value_destroy(myfloat);
}

//Double
TEST(testValue, testdouble)
{
  qi_value_t *mydouble = qi_value_create("d");
  qi_value_set_double(mydouble, 1000000000);

  double double_return;
  qi_value_get_double(mydouble, &double_return);
  ASSERT_TRUE(double_return  == 1000000000);

  double_return = qi_value_get_double_default(mydouble, double_return);
  ASSERT_TRUE(double_return  == 1000000000);
  qi_value_destroy(mydouble);
}

//String
TEST(testValue, teststring)
{
  qi_value_t *mystring = qi_value_create("s");
  qi_value_set_string(mystring, "test_string");

  ASSERT_TRUE(strcmp(qi_value_get_string(mystring), "test_string") == 0);
  qi_value_destroy(mystring);
}

//List
TEST(DISABLED_testValues, testlist)
{
  qi_value_t *mylist = qi_value_create("[s]");

  qi_value_t *mystring = qi_value_create("s");
  qi_value_set_string(mystring, "test_string");
  qi_value_t *otherstring = qi_value_create("s");
  qi_value_set_string(mystring, "second_string");

  qi_value_list_set(mylist, 0, mystring);
  qi_value_list_set(mylist, 1, otherstring);
  qi_value_list_set(mylist, 2, mystring);
  qi_value_list_set(mylist, 3, otherstring);


  unsigned int size = qi_value_list_size(mylist);

  printf("%d", qi_value_list_size(mylist));

  ASSERT_EQ(size, 4);

  int i;
  for(i = 0; i < size; i++)
  {
    const char *val = qi_value_get_string(qi_value_list_get(mylist, i));
    if(i % 2 == 0)//mystring else otherstring
    {
      ASSERT_TRUE(strcmp(val, "test_string") == 0);
    }
    else
    {
      ASSERT_TRUE(strcmp(val, "second_string") == 0);
    }
  }
  qi_value_destroy(mylist);
  qi_value_destroy(mystring);
  qi_value_destroy(otherstring);
}

//Map
TEST(TestValues, testMap)
{
  const char *msg;

  qi_value_t *map = qi_value_create("{Is}");

  qi_value_t *key = qi_value_create("i");
  qi_value_t *value = qi_value_create("s");

  qi_value_set_int64(key, 0);
  qi_value_set_string(value, "first_value");
  qi_value_map_set(map, key, value);

  qi_value_t *returned_value;
  returned_value = qi_value_map_get(map, key);

  msg = qi_value_get_string(returned_value);
  ASSERT_TRUE(strcmp(msg, "first_value") == 0);
  ASSERT_TRUE(qi_value_map_size(map) == 1);

  qi_value_destroy(map);
  qi_value_destroy(value);
  qi_value_destroy(key);
  qi_value_destroy(returned_value);
}

//Tuple
TEST(TestValues, testtuple)
{
  long long result;
  qi_value_t *mytuple = qi_value_create("(si)");

  qi_value_t *mystring = qi_value_create("s");
  qi_value_set_string(mystring, "test_string");
  qi_value_t *myint = qi_value_create("i");
  qi_value_set_int64(myint, 42);

  qi_value_tuple_set(mytuple, 0, mystring);
  qi_value_tuple_set(mytuple, 1, myint);

  ASSERT_EQ(qi_value_tuple_size(mytuple), 2);

  qi_value_t *tuple_element = qi_value_tuple_get(mytuple, 0);
  ASSERT_TRUE(strcmp(qi_value_get_string(tuple_element), "test_string") == 0);

  qi_value_t *second_element = qi_value_tuple_get(mytuple, 1);
  qi_value_get_int64(second_element, &result);
  ASSERT_TRUE(result == 42);

  qi_value_destroy(mytuple);
  qi_value_destroy(mystring);
  qi_value_destroy(myint);
  qi_value_destroy(tuple_element);
  qi_value_destroy(second_element);
}

TEST(testValues, testdyn)
{
  qi_value_t* mydyn = qi_value_create("m");
  qi_value_t* mystring = qi_value_create("s");

  //set the dynamic
  qi_value_set_string(mystring, "test_string");
  qi_value_dynamic_set(mydyn, mystring);

  mystring = qi_value_dynamic_get(mydyn);
  assert(mystring);

  assert(qi_value_get_string(mystring));

  assert(qi_value_get_string(qi_value_dynamic_get(mydyn)));

  const char *mystr = qi_value_get_string(qi_value_dynamic_get(mydyn));

  ASSERT_TRUE(mystr && strcmp(mystr, "test_string") == 0);
  free((void*)mystr);
  qi_value_destroy(mystring);
  qi_value_destroy(mydyn);
}

TEST(DISABLED_TestValues, testobject)
{
  qi_session_t *sd;
  const char            *addr;

  qi_object_builder_t* ob = qi_object_builder_create();

  sd = qi_session_create();
  qi_session_listen_standalone(sd, "tcp://127.0.0.1:0");


  qi_value_t *val = qi_session_endpoints(sd);
  qi_value_t *ele = qi_value_list_get(val, 0);

  addr = qi_value_get_string(ele);
  qi_value_destroy(ele);
  qi_value_destroy(val);

  //connection session
  printf("session address: %s\n", addr);
  qi_session_t*  session = qi_session_create();
  qi_future_t* fuc = qi_session_connect(session, addr);
  ASSERT_FALSE(qi_future_has_error(fuc, QI_FUTURETIMEOUT_INFINITE));
  qi_future_destroy(fuc);

  //build object
  qi_object_builder_advertise_method(ob, "spawn_obj::m()", &spawn_obj, 0);
  qi_object_builder_advertise_method(ob, "print::v(s)", &print, 0);
  qi_object_t* obj = qi_object_builder_get_object(ob);
  qi_object_builder_destroy(ob);

  //register service
  qi_future_t* fut_reg = qi_session_register_service(session, "first_service", obj);
  qi_future_wait(fut_reg, QI_FUTURETIMEOUT_INFINITE);


  //get the service then get an object calling spawn_obj.
  qi_object_t* first_service = qi_future_get_object(qi_session_get_service(session, "first_service"));
  qi_value_t* param = qi_value_create("()");
  qi_future_t* fu_call = qi_object_call(first_service, "spawn_obj", param);
  qi_future_wait(fu_call, QI_FUTURETIMEOUT_INFINITE);
  if(qi_future_has_value(fu_call, QI_FUTURETIMEOUT_INFINITE))
  {
    printf("Object got\n");
  }
  else
  {
    printf("FUTURE EMPTY\n");
  }
  qi_value_t *dynamic_containing_obj = qi_future_get_value(fu_call);
  printf("%d\n", qi_value_get_kind(dynamic_containing_obj));
  qi_value_t *value_containing_object = qi_value_dynamic_get(dynamic_containing_obj);
  printf("%d\n", qi_value_get_kind(value_containing_object));
  qi_object_t *myobject = qi_value_get_object(value_containing_object);
  qi_object_t* spawned_object = qi_future_get_object(fu_call);//spawned_object
  (void)myobject;
  (void)spawned_object;
}


TEST(DISABLED_TestValues, testraw)
{
  int size = 5;
  const char * result = (char*)malloc(sizeof(char) * 5);
  qi_value_t *myraw;

  myraw = qi_value_create("r");
  qi_value_raw_set(myraw, "toto", size);

  ASSERT_TRUE(strcmp(result, "toto") == 0);
  qi_value_destroy(myraw);
}

int main(int argc, char **argv) {
  qi_application_t*     app;
  app = qi_application_create(&argc, argv);
  ::testing::InitGoogleTest(&argc, argv);

  int r = RUN_ALL_TESTS();
  printf("destoyed\n");
  qi_application_destroy(app);
  return r;
}
