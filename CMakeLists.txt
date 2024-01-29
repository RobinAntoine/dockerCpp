cmake_minimum_required(VERSION 3.16)
project(dockercpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

file(GLOB_RECURSE SRC source/*.cpp)
file(GLOB_RECURSE HEADER include/*.h*)


add_executable(${PROJECT_NAME} ${SRC} ${HEADER})

target_link_libraries(${PROJECT_NAME} PUBLIC
    # add here all dep
    ${CURL_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    # Add include dir 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Add include dir from lib 
    ${CURL_INCLUDE_DIRS}
)