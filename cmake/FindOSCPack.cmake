# FindOSCPack.cmake
#
# Finds the oscpack library
#
# This will define the following variables
#
#    OSCPack_FOUND
#    OSCPack_INCLUDE_DIRS
#    OSCPack_LIBRARIES
#
# and the following imported targets
#
#     OSCPack::OSCPack
#

find_path(OSCPack_INCLUDE_DIR
	NAMES "osc/OscTypes.h"
	PATH_SUFFIXES oscpack
)

find_library(OSCPack_LIBRARY
	NAMES liboscpack.so
)

mark_as_advanced(OSCPack_FOUND OSCPack_INCLUDE_DIR OSCPack_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OSCPack
	REQUIRED_VARS OSCPack_INCLUDE_DIR OSCPack_LIBRARY
)

if(OSCPack_FOUND)
	set(OSCPack_INCLUDE_DIRS ${OSCPack_INCLUDE_DIR})
	set(OSCPack_LIBRARIES ${OSCPack_LIBRARY})
endif()

if(OSCPack_FOUND AND NOT TARGET OSCPack::OSCPack)
	add_library(OSCPack::OSCPack INTERFACE IMPORTED)
	set_target_properties(OSCPack::OSCPack PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${OSCPack_INCLUDE_DIR}"
		INTERFACE_LINK_LIBRARIES "${OSCPack_LIBRARY}"
	)
endif()

