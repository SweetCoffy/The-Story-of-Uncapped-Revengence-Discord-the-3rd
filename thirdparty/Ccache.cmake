# Enable CCache
# (Set USE_CCACHE=ON to use, CCACHE_OPTIONS for options)
if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL Windows)
	option(USE_CCACHE "Enable ccache support" OFF)

	if(USE_CCACHE)
		find_program(CCACHE_TOOL_PATH ccache)
		if(CCACHE_TOOL_PATH)
			set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_TOOL_PATH} CACHE STRING "" FORCE)
			set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_TOOL_PATH} CACHE STRING "" FORCE)
		else()
			message(WARNING "USE_CCACHE was set but ccache is not found (set CCACHE_TOOL_PATH)")
		endif()
	endif()
endif()