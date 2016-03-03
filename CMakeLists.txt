###############################################################################
# Graybat examples and tests
###############################################################################
cmake_minimum_required(VERSION 3.3.0)
project("graybat examples and tests")

###############################################################################
# Boost
###############################################################################
find_package(Boost 1.58.0 COMPONENTS unit_test_framework program_options REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/include/graybat/utils/hana/include/")
set(LIBS ${LIBS} ${Boost_LIBRARIES})

###############################################################################
# Elegant Progressbars for a More Cililized Age
###############################################################################
set(elegant-progressbars_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/elegant-progressbars/cmake")
find_package(elegant-progressbars 1.0.0 REQUIRED CONFIG)

###############################################################################
# Graybat
###############################################################################
find_package(graybat 1.0.0 REQUIRED CONFIG PATHS ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${graybat_INCLUDE_DIRS})
set(LIBS ${LIBS} ${graybat_LIBRARIES})

###############################################################################
# Compiler Flags
###############################################################################
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set  (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-parameter")
  #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-parameter")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wvla-extension")      
  #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

###############################################################################
# CCache
###############################################################################
#find_program(CCACHE_FOUND ccache)
#if(CCACHE_FOUND)
#  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
#  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
#endif(CCACHE_FOUND)

###############################################################################
# Targets
###############################################################################

# Test cases
file(GLOB TESTS test/*.cpp)
add_executable(check EXCLUDE_FROM_ALL ${TESTS})
target_link_libraries(check ${LIBS})

# Fast testing
file(GLOB TESTSFAST test/UT.cpp test/CommunicationPolicyUT.cpp)
add_executable(checkFast EXCLUDE_FROM_ALL ${TESTSFAST})
target_link_libraries(checkFast ${LIBS})


# Examples
add_custom_target(example)
file(GLOB EXAMPLES  example/*.cpp)
foreach(EXAMPLE ${EXAMPLES})
	get_filename_component(DEP ${EXAMPLE} NAME_WE)
	add_executable(${DEP} EXCLUDE_FROM_ALL ${EXAMPLE})
	target_link_libraries(${DEP} LINK_INTERFACE_LIBRARIES graybat)	
	target_link_libraries(${DEP} ${LIBS})	
	add_dependencies(example ${DEP})
endforeach(EXAMPLE)

# ZMQ signaling server
add_executable(gbZMQSignaling EXCLUDE_FROM_ALL utils/zmq_signaling_server.cpp )
target_link_libraries(gbZMQSignaling ${LIBS})

# CTest
enable_testing()
add_test(graybat_check_build "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target check)
add_test(graybat_example_build "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target example)
add_test(graybat_test_run  mpiexec --allow-run-as-root -n 2 check )
add_test(graybat_gol_run  mpiexec --allow-run-as-root gol 90 4 )
set_tests_properties(graybat_test_run PROPERTIES DEPENDS graybat_check_build)
set_tests_properties(graybat_gol_run PROPERTIES DEPENDS graybat_example_build)

# Install
file(GLOB CMAKE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include"
  DESTINATION "lib/graybat"
  )

install(
  FILES ${CMAKE_FILES}
  DESTINATION "lib/graybat"
  )

# Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc 
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen." VERBATIM
    )
endif(DOXYGEN_FOUND)

###############################################################################
# Clion Integration
###############################################################################
file(GLOB_RECURSE _SOURCES_PRE "*")
foreach(P ${_SOURCES_PRE})
    # Remove dot-directories and the build directory
    if(NOT ${P} MATCHES  "${CMAKE_CURRENT_LIST_DIR}/([.].*/.*|build/.*)")
        set(_SOURCES ${_SOURCES} ${P})
    endif()
endforeach()
ADD_LIBRARY(CLION_DUMMY_TARGET EXCLUDE_FROM_ALL ${_SOURCES})