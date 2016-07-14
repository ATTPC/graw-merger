cmake_minimum_required(VERSION 3.0)
project(graw2hdf)

# Define all of the files to be built and included

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DARMA_NO_DEBUG")

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-padded -Wno-weak-vtables -Wno-shadow -Wno-exit-time-destructors -Wno-documentation-unknown-command -Wno-missing-prototypes -Werror")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wfloat-equal -Wpointer-arith -Wcast-qual")
endif()

set(MERGER_FILES
    src/DataFile.cpp
    src/Event.cpp
    src/GRAWFile.cpp
    src/GRAWFrame.cpp
    src/LookupTable.cpp
    src/PadLookupTable.cpp
    src/Merger.cpp
    src/HDFDataStore.cpp
    src/FileIndex.cpp)

set(MAIN_FILE src/main.cpp)

include_directories(include)

find_package(Boost 1.50.0 REQUIRED COMPONENTS log date_time system filesystem program_options thread)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(Armadillo REQUIRED)

find_package(HDF5 REQUIRED COMPONENTS CXX)
include_directories(SYSTEM ${HDF5_INCLUDE_DIRS})

# Set up targets

add_executable(graw2hdf ${MERGER_FILES} ${MAIN_FILE})
target_link_libraries(graw2hdf ${Boost_LIBRARIES} pthread ${Armadillo_LIBRARIES} ${HDF5_CXX_LIBRARIES})

# Install

install(TARGETS graw2hdf DESTINATION bin)
