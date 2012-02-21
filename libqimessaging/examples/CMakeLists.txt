##
## Author(s):
##  - Cedric GESTES      <gestes@aldebaran-robotics.com>
##  - Chris Kilner       <ckilner@aldebaran-robotics.com>
##
## Copyright (C) 2009, 2010, 2011 Aldebaran Robotics
##


function(_qimessaging_create_example name)
  qi_create_bin(${name}
    ${name}.cpp
    NO_INSTALL
    DEPENDS
      QI
      QIMESSAGING
      BOOST_PROGRAM_OPTIONS
  )
  set_target_properties(${name}
    PROPERTIES
      FOLDER "examples")
endfunction()

#_qimessaging_create_example(example_qi_client             )
#_qimessaging_create_example(example_qi_server             )
#_qimessaging_create_example(example_qi_publisher          )
#_qimessaging_create_example(example_qi_subscriber         )
_qimessaging_create_example(example_qi_signature_type     )
_qimessaging_create_example(example_qi_signature_instance )
_qimessaging_create_example(example_qi_signature_pp       )
#_qimessaging_create_example(example_qi_signature_function )
_qimessaging_create_example(example_qi_signature_iterator )
