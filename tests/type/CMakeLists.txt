qi_create_gmock(
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
  qi

  TIMEOUT 30
)

if(QI_WITH_TESTS)
  qi_create_module(qi_test_anymodule SRC cat.hpp qi_test_anymodule.cpp SHARED DEPENDS QI NO_INSTALL)
  install(TARGETS qi_test_anymodule DESTINATION lib COMPONENT test)
endif()
