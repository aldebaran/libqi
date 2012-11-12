##
## Author(s):
##  - Pierre ROULLON        <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics

qi_add_test(test_dotnet_call      dotnetruntest.sh ARGUMENTS  "test_call"   "${CMAKE_CURRENT_SOURCE_DIR}" "${QI_SDK_DIR}"  TIMEOUT 5)
