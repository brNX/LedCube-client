CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(ledcube)

INCLUDE_DIRECTORIES( 
      ${PROJECT_BINARY_DIR}
)

INCLUDE(FindLibUSB.cmake)

#provide the source path so we can open our .cl file at runtime
ADD_DEFINITIONS(-Wall -std=gnu99)

set (CMAKE_BUILD_TYPE "Release")

set (CMAKE_C_FLAGS_DEBUG "-g -O0")
set (CMAKE_C_FLAGS_RELEASE "-march=native -O2")
set (CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set (CMAKE_CXX_FLAGS_RELEASE "-march=native -O2")


#create the executable
ADD_EXECUTABLE(ledcube hidtool.c hiddata.c)
TARGET_LINK_LIBRARIES (ledcube ${LibUSB_LIBRARIES})


