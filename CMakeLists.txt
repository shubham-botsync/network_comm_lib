# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.0)

project(bi_zones)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  #roscpp
  #roslib
  #rospy
  #std_msgs
  #geometry_msgs
  #bi_traffic_msgs
  #bi_hardware_msgs
  #bi_task_msgs
  #move_base_msgs
)


#set(Boost_INCLUDE_DIR /home/skpro19/boost_1_77_0)
#set(Boost_LIBRARY_DIR /home/skpro19/boost_1_77_0/lib)

## System dependencies are found with CMake's conventions
find_package(CURL REQUIRED)
#find_package(Boost REQUIRED COMPONENTS system)
#find_package(Boost REQUIRED COMPONENTS thread)
find_package(Boost REQUIRED COMPONENTS system)

###########
## Build ##
###########

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${boost_INCLUDE_DIRS}
)


add_executable(foo src/rest_api.cpp)


add_dependencies(foo ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

## Specify libraries to link a library or executable target against
target_link_libraries(foo
   ${catkin_LIBRARIES}
   ${CURL_LIBRARIES}
   yaml-cpp
   pthread
   boost_filesystem
   boost_system
   boost_thread
 )

