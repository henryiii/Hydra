cmake_minimum_required(VERSION 3.4)

# Project name
project(Hydra
    LANGUAGES CXX
    VERSION 2.0.0)

# Warn user if system is not UNIX
if(NOT UNIX)
    message(WARN "This is an unsupported system.")
endif()

set(Hydra_CMAKE_DIR "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "${Hydra_CMAKE_DIR}" ${CMAKE_MODULE_PATH})

# C++11 support required
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Users can link to this library or one of the _cpp, _omp, _tbb, or _cuda variants (special care needed for cuda)
add_library(Hydra INTERFACE)
target_link_libraries(Hydra INTERFACE ${CMAKE_DL_LIBS})

# Generator expressions
# True if in release mode
set(rel_mode "$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>")
# True if compiler is GCC
set(gcc_mode "$<CXX_COMPILER_ID:GNU>")
# True if release and GCC compiler
set(rel_gcc_mode "$<AND:${rel_mode},${gcc_mode}>")


# Just in case GCC gets higher performance with O4
target_compile_options(Hydra INTERFACE -ftemplate-backtrace-limit=0 -Wextra $<${rel_mode}:-march=native> $<${rel_gcc_mode}:-O4>)

# This removes the common warning that a parameter is unused.
# Due to the way Hydra has set signatures in some places,
# it is tempting to add a name to all parameters, causing this warning.
# You can leave unused parameters explicitly unnamed in most cases,
# but this is not likely to be worth warning the user about.
target_compile_options(Hydra INTERFACE -Wno-unused-parameter)

# Get TCLAP
# Empty target if TClap does not exist, use TCLAP_FOUND to check in CMake
find_package(TCLAP)
add_library(tclap_lib INTERFACE)
if(TCLAP_FOUND)
    message(STATUS "TCLAP include path: ${TCLAP_INCLUDE_DIR}")
    target_include_directories(tclap_lib INTERFACE "${TCLAP_INCLUDE_DIR}")
else()
    message(STATUS "TCLAP not found")
endif()

# Get ROOT
# Empty target if ROOT does not exist, use _ROOT_AVAILABLE_ to check in C++
find_package(ROOT COMPONENTS Minuit2)
add_library(ROOT_lib INTERFACE)
if(ROOT_FOUND)
    target_include_directories(ROOT_lib INTERFACE "${ROOT_INCLUDE_DIR}")
    target_link_libraries(ROOT_lib INTERFACE "${ROOT_LIBRARIES}")
    target_compile_options(ROOT_lib INTERFACE "-D_ROOT_AVAILABLE_")
    message(STATUS "ROOT include path: ${ROOT_INCLUDE_DIR}")
    message(STATUS "ROOT library path: ${ROOT_LIBRARY_DIR}" )
    message(STATUS "ROOT libraries: ${ROOT_LIBRARIES}")
endif(ROOT_FOUND)

# Get CUDA
find_package(CUDA 8.0)

# Get TBB
find_package(TBB)
if(TBB_FOUND)
    message(STATUS "TBB include: ${TBB_INCLUDE_DIRS}" )
    message(STATUS "TBB libraries: ${TBB_LIBRARIES}" )
endif()

# Get OpenMP
find_package(OpenMP)
if(OPENMP_CXX_FOUND OR OPENMP_FOUND)
    # Only need to set up OpenMP target on old CMakes, newer ones provide it
    if(NOT TARGET OpenMP::OpenMP_CXX)
        add_library(OpenMP_CXX_Target INTERFACE)
        add_library(OpenMP::OpenMP_CXX ALIAS OpenMP_CXX_Target)
        target_compile_options(OpenMP_CXX_Target INTERFACE "${OpenMP_CXX_FLAGS}")
        find_package(Threads REQUIRED)
        target_link_libraries(OpenMP_CXX_Target INTERFACE Threads::Threads)
        target_link_libraries(OpenMP_CXX_Target INTERFACE ${OpenMP_CXX_FLAGS})
    endif()
    message(STATUS "OpenMP flags: ${OpenMP_CXX_FLAGS}" )
    message(STATUS "OpenMP libraries:  ${OpenMP_CXX_LIBRARIES}" )
endif()


# Generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)

# Including directories
target_include_directories(Hydra INTERFACE ${PROJECT_SOURCE_DIR})

if(CUDA_FOUND)

	#set cuda flags
	set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -ftemplate-backtrace-limit=0; --cudart ;  static ; -O4 ;--expt-relaxed-constexpr; -fmad=true; --expt-extended-lambda;--relocatable-device-code=false ;
	-Xptxas -dlcm=ca; -Xptxas --opt-level=4; -std=c++11 )
    if(ROOT_FOUND)
        set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -D_ROOT_AVAILABLE_)
    endif()

    set(CUDA_PROPAGATE_HOST_FLAGS ON)
	set(CUDA_SEPARABLE_COMPILATION OFF)
	set(CUDA_VERBOSE_BUILD OFF)

	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8)
	   list(APPEND CUDA_NVCC_FLAGS " -Xcompiler -D_MWAITXINTRIN_H_INCLUDED ")
    endif()

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.4)
	    list(APPEND CUDA_NVCC_FLAGS " -Xcompiler -D__CORRECT_ISO_CPP11_MATH_H_PROTO ")
    endif()
	# Detect CUDA architecture and get best NVCC flags

	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindCudaArch.cmake)
	select_nvcc_arch_flags(NVCC_FLAGS_EXTRA)

	list(APPEND CUDA_NVCC_FLAGS ${NVCC_FLAGS_EXTRA})

    message(STATUS "CUDA include: ${CUDA_INCLUDE_DIRS}" )
    message(STATUS "CUDA libraries: ${CUDA_LIBRARIES}"  )
    message(STATUS "NVCC flags: ${CUDA_NVCC_FLAGS}" )

    add_library(Hydra_cuda INTERFACE)
    target_link_libraries(Hydra_cuda INTERFACE Hydra)
    add_library(Hydra::CUDA ALIAS Hydra_cuda)

endif(CUDA_FOUND)

add_library(Hydra_cpp INTERFACE)
add_library(Hydra::CPP ALIAS Hydra_cpp)
target_compile_options(Hydra_cpp INTERFACE -DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_CPP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_CPP)
target_link_libraries(Hydra_cpp INTERFACE Hydra)

if(TARGET OpenMP::OpenMP_CXX)
    add_library(Hydra_omp INTERFACE)
    add_library(Hydra::OMP ALIAS Hydra_omp)
    target_compile_options(Hydra_omp INTERFACE -DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_CPP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP)
    target_link_libraries(Hydra_omp INTERFACE Hydra OpenMP::OpenMP_CXX)
endif()

if(TBB_FOUND)
    add_library(Hydra_tbb INTERFACE)
    add_library(Hydra::TBB ALIAS Hydra_tbb)
    target_compile_options(Hydra_tbb INTERFACE -DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_CPP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_TBB)
    target_link_libraries(Hydra_tbb INTERFACE Hydra tbb)
endif()

option(BUILD_CPP_TARGETS "Build CPP backend targets" ON)

if(TARGET Hydra_omp)
    option(BUILD_OMP_TARGETS "Build OMP backend targets" ON)
else()
    option(BUILD_OMP_TARGETS "Build OMP backend targets" OFF)
endif()

if(TARGET Hydra_cuda)
    option(BUILD_CUDA_TARGETS "Build CPP backend targets" ON)
else()
    option(BUILD_CUDA_TARGETS "Build CUDA backend targets" OFF)
endif()

if(TARGET tbb)
    option(BUILD_TBB_TARGETS "Build TBB backend targets" ON)
else()
    option(BUILD_TBB_TARGETS "Build TBB backend targets" OFF)
endif()

# Nice messages over the status
message(STATUS "Build CUDA/NVCC-based targets: ${BUILD_CUDA_TARGETS}")
message(STATUS "Build TBB targets: ${BUILD_TBB_TARGETS}")
message(STATUS "Build OpenMP targets: ${BUILD_OMP_TARGETS}")

message(STATUS "Install diretory: ${CMAKE_INSTALL_PREFIX}"  )
message(STATUS "Project source dir: ${PROJECT_SOURCE_DIR}" )
message(STATUS "Project build dir: ${PROJECT_BINARY_DIR}" )

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/plots)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/plots/dalitz)


# This is primarily for examples, but users can also add their
# own packages with the commands provided in this file, too
include(${Hydra_CMAKE_DIR}/AddHydraExample.cmake)


# Examples
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(HYDRA_EXAMPLES "Build the Hydra examples" ON)
else()
    option(HYDRA_EXAMPLES "Build the Hydra examples" ON)
endif()

if(HYDRA_EXAMPLES)
    add_custom_target(examples)
    if(NOT TCLAP_FOUND)
        message(FATAL_ERROR "TCLAP is required for the examples!")
    endif()
    add_subdirectory(examples/phase_space)
    add_subdirectory(examples/numerical_integration)
    add_subdirectory(examples/random)
    add_subdirectory(examples/fit)
    add_subdirectory(examples/histograming)
    add_subdirectory(examples/async)
    add_subdirectory(examples/misc)
    add_subdirectory(examples/phys)
endif()


# Installing
install(DIRECTORY Hydra
  DESTINATION include)

install(DIRECTORY
	src
  DESTINATION src/hydra/samples)

install(DIRECTORY
	cmake
  DESTINATION src/hydra/cmake)

install(FILES
  CMakeLists.txt
  COPYING
  THANKS.md
  README.md
  CHANGELOG.md
  DESTINATION src/Hydra)


# Documentation
if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc DESTINATION src/hydra/doc)

endif()

# Build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set(CPACK_SOURCE_IGNORE_FILES  build/*)
set(CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README
     "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_VERSION_MAJOR "${HYDRA_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${HYDRA_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${HYDRA_VERSION_PATCH}")
include (CPack)



