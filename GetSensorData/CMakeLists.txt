cmake_minimum_required(VERSION 3.5)
project(getSensorData)

set(CMAKE_CXX_STANDARD 11)

set(CeleX ./)
set(CeleX_LIB CeleX)

include_directories(${CeleX}/include)
link_directories(${CeleX}/lib/Linux/x64)

if(CMAKE_VERSION VERSION_LESS "2.8.11")
    # Add OpenCV headers location to your include paths
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()


add_executable(getSensorData ./GetSensorData.cpp)

#specify the executable directory 
set_target_properties(getSensorData PROPERTIES OUTPUT_NAME ./build/GetSensorData)

# Link your application with CeleX librarie
target_link_libraries(getSensorData ${CeleX_LIB})

