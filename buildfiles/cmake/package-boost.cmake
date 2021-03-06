
if (NOT BLACKFIN)
  set(BOOST_ROOT ${RAM_ROOT_DIR})
  set(Boost_USE_MULTITHREADED OFF)
  find_package(Boost 1.34 REQUIRED COMPONENTS system filesystem date_time program_options python regex serialization signals thread)
  
  add_definitions(-Wno-deprecated)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif ()
