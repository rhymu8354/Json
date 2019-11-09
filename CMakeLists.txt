# CMakeLists.txt for Json
#
# © 2018 by Richard Walters

cmake_minimum_required(VERSION 3.8)
set(This Json)

set(Headers
    include/Json/Value.hpp
)

set(Sources
    src/Value.cpp
)

add_library(${This} STATIC ${Sources} ${Headers})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)

target_include_directories(${This} PUBLIC include)

target_link_libraries(${This} PUBLIC
    SystemAbstractions
    Utf8
)

add_subdirectory(test)
