cmake_minimum_required(VERSION 3.5)

project(IndexedSet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(IndexedSet2 main.cpp)

include(GNUInstallDirs)
install(TARGETS IndexedSet2
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
