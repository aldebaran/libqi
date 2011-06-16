project(qi_examples)

if (WITH_BOOST_LOCALE)
  create_bin(qipath_example qipath_example.cpp NO_INSTALL)
  use_lib(qipath_example QI BOOST_LOCALE)
  set_target_properties(qipath_example PROPERTIES FOLDER "examples")
endif()

create_bin(sharedlibrary_example sharedlibrary_example.cpp NO_INSTALL)
use_lib(sharedlibrary_example QI)
set_target_properties(sharedlibrary_example PROPERTIES FOLDER "examples")

if (UNIX AND WITH_BOOST_LOCALE)
	create_bin(qiconvloc_example qiconvloc_example.cpp NO_INSTALL)
	use_lib(qiconvloc_example QI BOOST_LOCALE)
	set_target_properties(qiconvloc_example PROPERTIES FOLDER "examples")
endif()
