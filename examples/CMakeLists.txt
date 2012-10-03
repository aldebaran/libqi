##
## Author(s):
##  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
##
## Copyright (C) 2012 Aldebaran Robotics
##

function(_qimessaging_create_example name)
  qi_create_bin(${name}
  ${name}.cpp
  NO_INSTALL
  DEPENDS
    QI
    QIPERF
    BOOST_PROGRAM_OPTIONS
  )
  set_target_properties(${name}
  PROPERTIES
    FOLDER "examples")
endfunction()

_qimessaging_create_example(example_qiperf)
qi_use_lib(example_qiperf QIPERF)
