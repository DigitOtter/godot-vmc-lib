set(PROJECT_NAME "VMCGodot")
set(HEADER_DIRECTORY "vmc_godot")

set(NAMESPACE_NAME "${PROJECT_NAME}")

set(LIBRARY_NAME "${PROJECT_NAME}")
set(EXECUTABLE_NAME "${PROJECT_NAME}Exec")
set(TEST_NAME "${PROJECT_NAME}Tests")

set(LIB_EXPORT_NAME "${LIBRARY_NAME}Targets")
set(LIB_CONFIG_NAME "${LIBRARY_NAME}Config")
set(LIB_VERSION_NAME "${LIB_CONFIG_NAME}Version")


cmake_minimum_required(VERSION 3.18)
project("${PROJECT_NAME}" VERSION 1.0.0)

include(GNUInstallDirs)

option(ARCHITECTURE "x64 or x86 CPU Architecture" "x64")

set (CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


########################################
## Godot C++ Headers
add_subdirectory("third_party/godot-cpp")
add_library(GodotCpp::GodotCpp ALIAS godot-cpp)


########################################
## OSCPack library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(third_party/oscpack)


##########################################
## Library
set(LIB_SRC_FILES
	vmc_godot/vmc_godot_impl.cpp
	vmc_godot/vmc_receiver.cpp
	vmc_godot/vmc_packets.cpp
)

add_library("${LIBRARY_NAME}" SHARED ${LIB_SRC_FILES})
add_library("${NAMESPACE_NAME}::${LIBRARY_NAME}" ALIAS ${LIBRARY_NAME})
target_compile_options(${LIBRARY_NAME} PUBLIC $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wextra>)

target_include_directories(${LIBRARY_NAME}
	PUBLIC
	PRIVATE
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

target_link_libraries(${LIBRARY_NAME}
	PUBLIC
	PRIVATE
		OSCPack::OSCPack
		GodotCpp::GodotCpp
)
