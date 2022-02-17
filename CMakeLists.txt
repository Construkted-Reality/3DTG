cmake_minimum_required(VERSION 3.10)

# 3D Tiles Generator
set(TARGET_NAME 3DTG)

set(3DTG_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(3DTG_DIR_INC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(3DTG_DIR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)

project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_definitions("-std=c++17")
#add_definitions("-pthread")
#SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Werror -O3 -std=c++17 -pthread")

find_package(Threads)

include_directories(${3DTG_DIR_INC}/)
link_directories(${3DTG_LIBS}/)

file(GLOB_RECURSE 
    SOURCE_FILES 
    ${3DTG_DIR_INC}/*.h 
    ${3DTG_DIR_SRC}/*.cpp
)

IF (WIN32)
    file(GLOB DEP_LIBS ${3DTG_LIBS}/*.lib)
ELSE()
    file(GLOB DEP_LIBS ${3DTG_LIBS}/*.a)
ENDIF()

#message("Libs: " ${DEP_LIBS})

#target_include_directories (${3DTG_DIR_INC} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable (${TARGET_NAME} 
    ${3DTG_DIR_SRC}/main.cpp
    ${SOURCE_FILES}
)

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

target_link_libraries(${TARGET_NAME} ${DEP_LIBS} ${CMAKE_THREAD_LIBS_INIT})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)