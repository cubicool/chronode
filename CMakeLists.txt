cmake_minimum_required(VERSION 3.16.3)

project(chronode)

# -----------------------------------------------------------------------------
if(isMultiConfig)
	if(NOT "Asan" IN_LIST CMAKE_CONFIGURATION_TYPES)
		list(APPEND CMAKE_CONFIGURATION_TYPES Asan)
	endif()

else()
	set(allowedBuildTypes Asan Debug Release RelWithDebInfo MinSizeRel)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")

	if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
		message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
	endif()
endif()

set(CMAKE_C_FLAGS_ASAN
	"${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer" CACHE STRING
	"Flags used by the C compiler for Asan build type or configuration." FORCE
)

set(CMAKE_CXX_FLAGS_ASAN
	"${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer" CACHE STRING
	"Flags used by the C++ compiler for Asan build type or configuration." FORCE
)

set(CMAKE_EXE_LINKER_FLAGS_ASAN
	"${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address" CACHE STRING
	"Linker flags to be used to create executables for Asan build type." FORCE
)

set(CMAKE_SHARED_LINKER_FLAGS_ASAN
	"${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address" CACHE STRING
	"Linker lags to be used to create shared libraries for Asan build type." FORCE
)
# -----------------------------------------------------------------------------

if(WIN32)
	set(CHRONODE_COMPILE_OPTIONS
		"/W4"
		"/WX"
		"/wd4100"
		"/analyze"
		# "/wd4225"
		# "/wd4244"
		# "/wd4668"
		# "/wd4146"
		# "/wd4267"
		# "/wd4996"
		# "/wd4334"
		# "/wd4311"
	)

else(WIN32)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(CHRONODE_COMPILE_OPTIONS
			"-W"
			"-Wall"
			"-Wshadow"
			"-Wconversion"
			"-Wpedantic"
			"-Werror"
			"-Wno-unused-parameter"
			"-DCHRONODE_ENABLED"
		)
	else(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(CHRONODE_COMPILE_OPTIONS
			"-W"
			"-Wall"
			"-Wshadow"
			"-Wconversion"
			"-Wpedantic"
			"-Werror"
			"-Wno-unused-parameter"
			# "-fmessage-length=80"
			"-fmax-errors=1"
			# "-fcoroutines"
			# "-D_FORTIFY_SOURCE"
			"-DCHRONODE_ENABLED"
		)
	endif()
endif()

# set(CHRONODE_TESTS "node" "timer" "timer-macro")
set(CHRONODE_TESTS "node" "timer" "timer-macro")

# list(APPEND CHRONODE_TESTS
# 	(
# 		"timer-macro-multifile"
# 		"timer-macro-multifile0"
# 		"timer-macro-multifile1"
# 	)
# )

foreach(test IN LISTS CHRONODE_TESTS)
	add_executable("test-${test}" "chronode.hpp" "test/test-${test}.cpp")
	target_compile_features("test-${test}" PUBLIC cxx_std_17)
	target_include_directories("test-${test}" PRIVATE ${CMAKE_SOURCE_DIR})
	target_compile_options("test-${test}" PRIVATE ${CHRONODE_COMPILE_OPTIONS})
endforeach()

# add_executable(test-timer-macro-multifile "chronode.hpp"
# 	"test/test-timer-macro-multifile.cpp"
# 	"test/test-timer-macro-multifile0.cpp"
# 	"test/test-timer-macro-multifile1.cpp"
# )
# target_compile_features(test-timer-macro-multifile PUBLIC cxx_std_17)
# target_include_directories(test-timer-macro-multifile PRIVATE ${CMAKE_SOURCE_DIR})
# target_compile_options(test-timer-macro-multifile PRIVATE ${CHRONODE_COMPILE_OPTIONS})
