###
# Simple project to test that naoqiclient is usable

cmake_minimum_required(VERSION 2.4.6)
project(testippc)
include(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.cmake)


create_bin(testippc ippctest.cpp NO_INSTALL)


use_lib(testippc PROXIES LIBIPPC LIBCORE ALLOG LIBFINDIPPC LIBTHREAD BOOST BOOST_THREAD )

