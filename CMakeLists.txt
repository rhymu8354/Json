# CMakeLists.txt for Json
#
# © 2018 by Richard Walters

cmake_minimum_required(VERSION 3.8)
set(This Json)

set(Headers
    include/Json/Value.hpp
    include/Json/WebToken.hpp
)

set(Sources
    src/Value.cpp
    src/WebToken.cpp
)

add_library(${This} STATIC ${Sources} ${Headers})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)

target_include_directories(${This} PUBLIC include)

target_link_libraries(${This} PUBLIC
    Base64
    SystemAbstractions
    Utf8
)

add_subdirectory(test)
