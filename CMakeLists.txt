# copyright defined in abieos/LICENSE.txt

cmake_minimum_required (VERSION 3.8...3.31)
project(library_abi VERSION 0.1 LANGUAGES CXX C)

set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

option(ABIEOS_NO_INT128 "disable use of __int128" OFF)
option(ABI_ONLY_LIBRARY "define and build the ABIEOS library" OFF)

set(LIB_ABI_NAME "flon_abi")
set(LIB_MODULE_NAME ${LIB_ABI_NAME}_module )
set(TEST_EXE_NAME test_${LIB_ABI_NAME} )

if(NOT DEFINED SKIP_SUBMODULE_CHECK)
  execute_process(COMMAND git submodule status --recursive
                  COMMAND grep -c "^[+\-]"
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                  OUTPUT_VARIABLE submodule_status
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(submodule_status GREATER 0)
     message(FATAL_ERROR "git submodules are not up to date.
Please run the command 'git submodule update --init --recursive'.")
  endif()
endif()

find_package(Threads)
include(GNUInstallDirs)

add_library(${LIB_ABI_NAME} STATIC src/abi.cpp src/crypto.cpp include/eosio/fpconv.c)
target_include_directories(${LIB_ABI_NAME} PUBLIC
                          "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include"
                          "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/external/rapidjson/include"
                          "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

if(ABIEOS_NO_INT128)
target_compile_definitions(${LIB_ABI_NAME} PUBLIC ABIEOS_NO_INT128)
endif()

add_library(${LIB_MODULE_NAME} MODULE src/abieos.cpp src/abi.cpp src/crypto.cpp include/eosio/fpconv.c)
target_include_directories(${LIB_MODULE_NAME} PUBLIC
                          "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include;"
                          "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/external/rapidjson/include"
                          "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(${LIB_MODULE_NAME} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(${LIB_MODULE_NAME} PROPERTIES OUTPUT_NAME "${LIB_ABI_NAME}")

enable_testing()

add_executable(${TEST_EXE_NAME} src/test.cpp src/abieos.cpp src/ship.abi.cpp)
target_link_libraries(${TEST_EXE_NAME} ${LIB_ABI_NAME} ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME ${TEST_EXE_NAME} COMMAND ${TEST_EXE_NAME})

if(NOT ABIEOS_NO_INT128)
    add_executable(${TEST_EXE_NAME}_template src/template_test.cpp src/abieos.cpp)
    target_link_libraries(${TEST_EXE_NAME}_template ${LIB_ABI_NAME} ${CMAKE_THREAD_LIBS_INIT})
    add_test(NAME ${TEST_EXE_NAME}_template COMMAND ${TEST_EXE_NAME}_template)
endif()

add_executable(${TEST_EXE_NAME}_key src/key_test.cpp src/abieos.cpp)
target_link_libraries(${TEST_EXE_NAME}_key ${LIB_ABI_NAME} ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME ${TEST_EXE_NAME}_key COMMAND ${TEST_EXE_NAME}_key)

add_executable(${TEST_EXE_NAME}_reflect src/reflect_test.cpp)
target_include_directories(${TEST_EXE_NAME}_reflect PRIVATE include)
add_test(NAME ${TEST_EXE_NAME}_reflect COMMAND ${TEST_EXE_NAME}_reflect)

# Causes build issues on some platforms
# add_executable(test_abieos_sanitize src/test.cpp src/abieos.cpp src/abi.cpp src/crypto.cpp include/eosio/fpconv.c)
# target_include_directories(test_abieos_sanitize PRIVATE include external/outcome/single-header external/rapidjson/include external/date/include)
# target_link_libraries(test_abieos_sanitize abieos -fno-omit-frame-pointer -fsanitize=address,undefined ${CMAKE_THREAD_LIBS_INIT})
# target_compile_options(test_abieos_sanitize PUBLIC -fno-omit-frame-pointer -fsanitize=address,undefined)
# add_test(NAME test_abieos_sanitize COMMAND test_abieos_sanitize)

# add_executable(fuzzer src/fuzzer.cpp src/abieos.cpp)
# target_include_directories(fuzzer PRIVATE external/outcome/single-header external/rapidjson/include external/date/include)
# target_link_libraries(fuzzer -fsanitize=fuzzer,address,undefined,signed-integer-overflow -fstandalone-debug ${CMAKE_THREAD_LIBS_INIT})
# target_compile_options(fuzzer PRIVATE -fsanitize=fuzzer,address,undefined,signed-integer-overflow -fstandalone-debug)

if (CMAKE_CXX_COMPILER_ID MATCHES Clang|AppleClang)
    target_compile_options(${LIB_ABI_NAME} PRIVATE -Wall -Wextra -Wno-unused-parameter -fcolor-diagnostics)
    target_compile_options(${TEST_EXE_NAME} PRIVATE -Wall -Wextra -Wno-unused-parameter -fcolor-diagnostics)
endif()

if (NOT ABI_ONLY_LIBRARY)
add_subdirectory(tools)
endif()

if(LIB_ABI_INSTALL_COMPONENT)
   set(INSTALL_COMPONENT_ARGS COMPONENT ${LIB_ABI_INSTALL_COMPONENT} EXCLUDE_FROM_ALL)
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/eosio DESTINATION include ${INSTALL_COMPONENT_ARGS})

install(TARGETS ${LIB_ABI_NAME}
  EXPORT ${LIB_ABI_NAME}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} ${INSTALL_COMPONENT_ARGS}
	ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR} ${INSTALL_COMPONENT_ARGS}
)

install(EXPORT ${LIB_ABI_NAME}
  DESTINATION "share/${LIB_ABI_NAME}"
  FILE ${LIB_ABI_NAME}-targets.cmake
  ${INSTALL_COMPONENT_ARGS}
)

export(TARGETS ${LIB_ABI_NAME}
       FILE ${CMAKE_CURRENT_BINARY_DIR}/share/${LIB_ABI_NAME}/${LIB_ABI_NAME}-targets.cmake)
configure_file(lib_abi-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/share/${LIB_ABI_NAME}/${LIB_ABI_NAME}-config.cmake COPYONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/share/${LIB_ABI_NAME}/${LIB_ABI_NAME}-config.cmake
        DESTINATION "share/${LIB_ABI_NAME}"
        ${INSTALL_COMPONENT_ARGS})