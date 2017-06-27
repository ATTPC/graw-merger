cmake_minimum_required(VERSION 3.0)
project(graw2hdf)

# Define all of the files to be built and included

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DARMA_NO_DEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DARMA_NO_DEBUG")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wfloat-equal -Wpointer-arith -Wcast-qual")

set(MERGER_FILES
    src/DataFile.cpp
    src/Event.cpp
    src/GRAWFile.cpp
    src/GRAWFrame.cpp
    src/Merger.cpp
    src/HDFDataStore.cpp
    src/FileIndex.cpp)

set(MAIN_FILE src/main.cpp)

include_directories(include)

add_definitions(-DBOOST_ALL_DYN_LINK)
set(Boost_USE_STATIC_LIBS OFF) 

if(APPLE)
   set(Boost_USE_MULTITHREADED ON)  
   set(Boost_USE_STATIC_RUNTIME OFF) 
endif()

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
find_package(Boost 1.55.0 REQUIRED COMPONENTS log date_time system filesystem program_options thread) 


find_package(Armadillo REQUIRED)

find_package(HDF5 REQUIRED COMPONENTS CXX)
include_directories(SYSTEM ${HDF5_INCLUDE_DIRS})

# Set up targets

add_executable(graw2hdf ${MERGER_FILES} ${MAIN_FILE})
target_link_libraries(graw2hdf ${Boost_LIBRARIES} ${Armadillo_LIBRARIES} ${HDF5_CXX_LIBRARIES})

# Install

install(TARGETS graw2hdf DESTINATION bin)
