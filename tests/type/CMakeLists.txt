qi_create_test(
  test_type

  SRC
  "test_object.hpp"
  "test_anymodule.cpp"
  "test_async.cpp"
  "test_clocktype.cpp"
  "test_event.cpp"
  "test_json.cpp"
  "test_metaobject.cpp"
  "test_object.cpp"
  "test_object_eventloop.cpp"
  "test_property.cpp"
  "test_proxysignal.cpp"
  "test_signal.cpp"
  "test_signature.cpp"
  "test_type.cpp"
  "test_value.cpp"
  "test_optional.cpp"

  DEPENDS
  qi::qi
  qi::testutils
  GTest::gmock

  TIMEOUT 30
)

qi_create_module(
  qi_test_anymodule
  SRC
    cat.hpp
    qi_test_anymodule.cpp
  NO_INSTALL
)
add_dependencies(test_type qi_test_anymodule)
