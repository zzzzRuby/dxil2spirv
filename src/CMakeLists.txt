if (MSVC)
	add_compile_options(
		/wd4146
		/wd4180
		/wd4244
		/wd4258
		/wd4267
		/wd4291
		/wd4345
		/wd4351
		/wd4355
		/wd4456
		/wd4457
		/wd4458
		/wd4459
		/wd4503
		/wd4624
		/wd4722
		/wd4800
		/wd4100
		/wd4127
		/wd4512
		/wd4505
		/wd4610
		/wd4510
		/wd4702
		/wd4245
		/wd4706
		/wd4310
		/wd4701
		/wd4703
		/wd4389
		/wd4611
		/wd4805
		/wd4204
		/wd4324
		/wd4244
		/wd4819
		/wd4146)
endif()
	
add_definitions(-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -D_SCL_SECURE_NO_WARNINGS)

file(GLOB_RECURSE SPIRV_BUILDER_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Builder/*)

set(dxil2spirv_src
  ${SPIRV_BUILDER_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/dxil2spirv.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/dxil2spirv_optimize.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/dxil2spirv_options.c)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${dxil2spirv_src})

add_library(dxil2spirv STATIC ${dxil2spirv_src})

add_executable(dxil-cross main.cpp)
target_link_libraries(dxil2spirv SPIRV-Tools-opt)
target_link_libraries(dxil-cross dxil2spirv)

if (SKULD)
  target_link_libraries(dxil2spirv DirectXShaderCompiler)
  skuld_mark_as_tool(dxil-cross)
  strip_symbol(dxil-cross)
else()
  target_link_libraries(dxil2spirv LLVMDXIL LLVMDxilContainer)
  target_link_libraries(dxil-cross LLVMMSSupport)
  target_include_directories(dxil-cross PRIVATE ${DXC_INCLUDE_DIR})
  target_include_directories(dxil2spirv PRIVATE ${SPIRV_HEADER_INCLUDE_DIR} ${SPIRV_TOOLS_INCLUDE_DIR} ${DXC_INCLUDE_DIR})
endif()