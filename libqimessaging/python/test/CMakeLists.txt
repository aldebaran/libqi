##
## Author(s):
##  - Pierre ROULLON        <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics

qi_add_test(test_python_bindings  pythonruntest.sh ARGUMENTS  "${QI_SDK_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../" "${CMAKE_CURRENT_SOURCE_DIR}/test_bindings.py"  TIMEOUT 5)
qi_add_test(test_python_proxy     pythonruntest.sh ARGUMENTS  "${QI_SDK_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../" "${CMAKE_CURRENT_SOURCE_DIR}/test_proxy.py"     TIMEOUT 5)
qi_add_test(test_python_call      pythonruntest.sh ARGUMENTS  "${QI_SDK_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../" "${CMAKE_CURRENT_SOURCE_DIR}/test_call.py"      TIMEOUT 5)
