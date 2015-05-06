cmake_minimum_required(VERSION 3.0)
project(get-manip)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Define all of the files to be built and included

set(LIBRARY_FILES
    get-manip/DataFile.cpp
    get-manip/Event.cpp
    get-manip/EventFile.cpp
    get-manip/GRAWDataItem.cpp
    get-manip/GRAWFile.cpp
    get-manip/GRAWFrame.cpp
    get-manip/LookupTable.cpp
    get-manip/PadLookupTable.cpp
    get-manip/Trace.cpp)

set(MERGER_FILES
    get-manip/Merger.cpp
    get-manip/UserInterface.cpp)

set(MAIN_FILE get-manip/main.cpp)

set(TEST_FILES
    get-manip-tests/EventTests.cpp
    get-manip-tests/FakeRawFrame.cpp
    get-manip-tests/GRAWFrameTests.cpp
    get-manip-tests/PadLookupTableTests.cpp
    get-manip-tests/TraceTests.cpp)

set(TEST_MAIN get-manip-tests/main.cpp)

include_directories(get-manip)

find_package(Boost 1.50.0 REQUIRED COMPONENTS date_time system filesystem program_options log)
include_directories(${Boost_INCLUDE_DIRS})

# Build googletest

set(GTEST_ROOT modules/gtest-1.7.0)
add_subdirectory(${GTEST_ROOT})
set(GTEST_INCLUDE_DIR ${GTEST_ROOT}/src)
include_directories(${GTEST_INCLUDE_DIR})
set(GTEST_LIBRARY ${CMAKE_BINARY_DIR}/${GTEST_ROOT}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX})

# Set up targets

add_library(getevt STATIC ${LIBRARY_FILES})
target_link_libraries(getevt ${Boost_LIBRARIES})

add_executable(get-manip ${MERGER_FILES} ${MAIN_FILE})
target_link_libraries(get-manip ${Boost_LIBRARIES} getevt)

add_executable(get-manip-tests ${SOURCE_FILES} ${TEST_FILES} ${TEST_MAIN})
target_link_libraries(get-manip-tests ${Boost_LIBRARIES} gtest getevt)