# copyright defined in abieos/LICENSE.txt

find_package(Threads)

add_executable(name name.cpp)
target_link_libraries(name ${LIB_ABI_NAME} ${CMAKE_THREAD_LIBS_INIT})

add_library(${LIB_ABI_NAME}_util STATIC ../src/abieos.cpp ../src/abi.cpp ../src/crypto.cpp ../include/eosio/fpconv.c)
target_include_directories(${LIB_ABI_NAME}_util PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/../src"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/../include"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/../external/rapidjson/include"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

add_executable(generate_hex_from_json util_generate_hex_from_json.cpp)
target_link_libraries(generate_hex_from_json ${LIB_ABI_NAME}_util ${CMAKE_THREAD_LIBS_INIT})

add_executable(generate_json_from_hex util_generate_json_from_hex.cpp)
target_link_libraries(generate_json_from_hex ${LIB_ABI_NAME}_util ${CMAKE_THREAD_LIBS_INIT})

add_custom_command( TARGET name POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE:name> ${CMAKE_CURRENT_BINARY_DIR}/name2num )
add_custom_command( TARGET name POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE:name> ${CMAKE_CURRENT_BINARY_DIR}/num2name )
