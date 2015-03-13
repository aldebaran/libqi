
qi_create_gtest(test_value            SRC test_value.cpp            DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_event            SRC test_event.cpp            DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_object           SRC test_object.cpp           DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_signature        SRC test_signature.cpp        DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_object_eventloop SRC test_object_eventloop.cpp DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_signal           SRC test_signal.cpp           DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_property         SRC test_property.cpp         DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_json             SRC test_json.cpp             DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_clocktype        SRC test_clocktype.cpp        DEPENDS QI GTEST TIMEOUT 30)
qi_create_gtest(test_anymodule        SRC test_anymodule.cpp        DEPENDS QI GTEST TIMEOUT 30)

qi_create_module(qi_test_anymodule    SRC cat.hpp qi_test_anymodule.cpp SHARED DEPENDS QI NO_INSTALL)
install(TARGETS qi_test_anymodule DESTINATION lib COMPONENT test)
