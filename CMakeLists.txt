cmake_minimum_required(VERSION 3.0)
project(get-manip)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2")

# Define all of the files to be built and included

set(MERGER_FILES
    get-manip/DataFile.cpp
    get-manip/Event.cpp
    get-manip/GRAWFile.cpp
    get-manip/GRAWFrame.cpp
    get-manip/LookupTable.cpp
    get-manip/PadLookupTable.cpp
    get-manip/Trace.cpp
    get-manip/Merger.cpp
    get-manip/UserInterface.cpp)

set(MAIN_FILE get-manip/main.cpp)

include_directories(get-manip)

find_package(Boost 1.50.0 REQUIRED COMPONENTS date_time system filesystem program_options log)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Armadillo REQUIRED)

# Set up targets

add_executable(get-manip ${MERGER_FILES} ${MAIN_FILE})
target_link_libraries(get-manip ${Boost_LIBRARIES} pthread ${Armadillo_LIBRARIES})

# Install

install(TARGETS get-manip DESTINATION bin)
