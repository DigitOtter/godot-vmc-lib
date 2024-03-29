set(PROJECT_NAME "GodotVMCLib")
set(HEADER_DIRECTORY "godot_vmc_lib")

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
set(VMC_LIB_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin" CACHE PATH "Install directory for generated lib")

set (CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(MINGW)
	string(APPEND CMAKE_SHARED_LINKER_FLAGS " -static -static-libgcc -static-libstdc++ -Wl,-allow-multiple-definition ")
endif()

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
	godot_vmc_lib/vmc_godot.cpp
	godot_vmc_lib/vmc_receiver.cpp
	godot_vmc_lib/vmc_packets.cpp
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

if(WIN32)
	set(INSTALL_CTRL
		RUNTIME DESTINATION ${VMC_LIB_INSTALL_DIR})
else()
	set(INSTALL_CTRL
		LIBRARY DESTINATION ${VMC_LIB_INSTALL_DIR})
endif()

install(TARGETS ${LIBRARY_NAME}
		EXPORT ${LIB_EXPORT_NAME}
		${INSTALL_CTRL}
)
