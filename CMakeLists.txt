set(PROJECT_NAME "GodotVMCLib")
set(NAMESPACE_NAME "${PROJECT_NAME}")

set(GODOT_LIB_NAME "${PROJECT_NAME}")
set(GODOT_DIR_NAME "godot-vmc-lib")

set(CMAKE_INSTALL_PREFIX
    "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    CACHE PATH "Library install location")
set(INSTALL_FILENAME
    "${GODOT_LIB_NAME}"
    CACHE STRING "Library name")
set(GD_RES_PATH
    "res://addons/${GODOT_DIR_NAME}/bin"
    CACHE STRING "Godot resource path")

cmake_minimum_required(VERSION 3.18)
project("${PROJECT_NAME}" VERSION 1.0.0)


########################################
## Godot C++ Headers
add_subdirectory("third_party/godot-cpp")
add_library(GodotCpp::GodotCpp ALIAS godot-cpp)

########################################
## GdExtension
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gd_library_data.gdextension.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${GODOT_DIR_NAME}.gdextension.in" @ONLY)

########################################
## OSCPack library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(third_party/oscpack)


##########################################
## Library
set(LIB_SRC_FILES
	godot_vmc_lib/vmc_godot.cpp
	godot_vmc_lib/vmc_receiver.cpp
	godot_vmc_lib/vmc_packets.cpp
)

add_library("${GODOT_LIB_NAME}" SHARED ${LIB_SRC_FILES})
add_library("${NAMESPACE_NAME}::${GODOT_LIB_NAME}" ALIAS ${GODOT_LIB_NAME})
target_compile_options(
    ${GODOT_LIB_NAME}
    PUBLIC $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall
           -Wextra>)
set_target_properties(
    ${GODOT_LIB_NAME} PROPERTIES LIBRARY_OUTPUT_NAME "${INSTALL_FILENAME}"
                                 PREFIX "")

target_include_directories(${GODOT_LIB_NAME}
	PUBLIC
	PRIVATE
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

target_link_libraries(${GODOT_LIB_NAME}
	PUBLIC
	PRIVATE
		OSCPack::OSCPack
		GodotCpp::GodotCpp
)


# ##############################################################################
# Install
install(
    TARGETS ${GODOT_LIB_NAME}
    LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}")

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${GODOT_DIR_NAME}.gdextension.in"
    DESTINATION "${CMAKE_INSTALL_PREFIX}"
    RENAME "${GODOT_DIR_NAME}.gdextension")
