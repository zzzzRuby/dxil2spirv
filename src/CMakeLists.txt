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
	add_definitions(-D_HAS_AUTO_PTR_ETC=1 -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif()
	
add_definitions(-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -D_SCL_SECURE_NO_WARNINGS)

file(GLOB_RECURSE SPIRV_BUILDER_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Builder/*)
file(GLOB_RECURSE DXIL_TRANSLATOR_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/DXIL-Translator/*)

set(dxil2spirv_src
  ${SPIRV_BUILDER_SOURCES}
  ${DXIL_TRANSLATOR_SOURCES}
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
  if (SKULD_BUILD_TESTS AND TARGET ShaderCrossCompiler)
	add_executable(dxil2spirv_test dxil2spirv_test.cpp)
	skuld_mark_as_functional_test(dxil2spirv_test)
  endif()
else()
  target_link_libraries(dxil2spirv LLVMDXIL LLVMDxilContainer)
  target_link_libraries(dxil-cross LLVMMSSupport)
  target_include_directories(dxil-cross PRIVATE ${DXC_INCLUDE_DIR})
  target_include_directories(dxil2spirv PRIVATE ${SPIRV_HEADER_INCLUDE_DIR} ${SPIRV_TOOLS_INCLUDE_DIR} ${DXC_INCLUDE_DIR})
  add_executable(dxil2spirv_test dxil2spirv_test.cpp)
endif()

if (TARGET dxil2spirv_test)
  find_package(PythonInterp 3 REQUIRED)
  set(DXIL2SPIRV_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../test)
  set(DXIL2SPIRV_TEST_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/../test)

  file(GLOB_RECURSE DXIL2SPIRV_ALL_TEST_HLSL CONFIGURE_DEPENDS ${DXIL2SPIRV_TEST_SOURCE_DIR}/*.hlsl)

  add_custom_command(OUTPUT ${DXIL2SPIRV_TEST_BINARY_DIR}/dxil2spirv_test.h
	COMMAND ${PYTHON_EXECUTABLE} ${DXIL2SPIRV_TEST_SOURCE_DIR}/compile_all.py
	  --input ${DXIL2SPIRV_TEST_SOURCE_DIR}
	  --output ${DXIL2SPIRV_TEST_BINARY_DIR}
	  --generate-header
	DEPENDS ${DXIL2SPIRV_ALL_TEST_HLSL} ${DXIL2SPIRV_TEST_SOURCE_DIR}/compile_all.py)
  target_link_libraries(dxil2spirv_test dxil2spirv)
  target_include_directories(dxil2spirv_test PRIVATE ${DXIL2SPIRV_TEST_BINARY_DIR})
  target_sources(dxil2spirv_test PRIVATE ${DXIL2SPIRV_TEST_BINARY_DIR}/dxil2spirv_test.h)

  if (NOT SKULD)
    target_link_libraries(dxil2spirv_test dxcompiler)
    add_custom_command(TARGET dxil2spirv_test POST_BUILD
	  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:dxcompiler> $<TARGET_FILE_DIR:dxil2spirv_test>)
  else()
	target_link_libraries(dxil2spirv_test ShaderCrossCompiler)
  endif()
endif()