cmake_minimum_required(VERSION 2.8.12.2)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-Dllvm=dxc_llvm -Dclang=dxc_clang)

if (NOT SKULD)
	add_subdirectory(third_party)
endif()

include_directories(include)
add_subdirectory(src)

target_include_directories(dxil2spirv INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)