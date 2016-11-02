cmake_minimum_required(VERSION 2.8)

project(HydraExamplePhSp)

SET(Hydra_CMAKE_DIR "${PROJECT_SOURCE_DIR}/cmake")
SET(CMAKE_MODULE_PATH "${Hydra_CMAKE_DIR}" ${CMAKE_MODULE_PATH})

include_directories(${PROJECT_SOURCE_DIR}/include)

find_package(ROOT)
if(ROOT_FOUND)
    include_directories(${ROOT_INCLUDE_DIR})
    link_directories(${ROOT_LIBRARY_DIR})
    add_definitions(-DROOT_FOUND)
endif()
    

find_package(CUDA 7.5)
if(CUDA_FOUND)
	SET(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -std=c++11; -I${CMAKE_CURRENT_SOURCE_DIR}/include --cudart ;  static ; -O4 ;--expt-relaxed-constexpr; -ftemplate-backtrace-limit=0; --expt-extended-lambda;--relocatable-device-code=false ; --generate-line-info ;
	 -Xptxas -fmad=true ;-Xptxas -dlcm=cg; -Xptxas --opt-level=4 ) #) 
# 
	SET(CUDA_SEPARABLE_COMPILATION OFF)
	SET(CUDA_VERBOSE_BUILD ON)
	
	# Detect CUDA architecture and get best NVCC flags
	
	INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindCudaArch.cmake)
	
	SELECT_NVCC_ARCH_FLAGS(NVCC_FLAGS_EXTRA)

	LIST(APPEND CUDA_NVCC_FLAGS ${NVCC_FLAGS_EXTRA})
	#hack for gcc 5.x.x
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
	    LIST(APPEND CUDA_NVCC_FLAGS " -D_MWAITXINTRIN_H_INCLUDED ")
    endif()

    cuda_add_executable(PhSpCuda
        src/PhSp.cu
    OPTIONS -Xcompiler -fopenmp -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_CUDA  -DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_OMP -lgomp
    )
    
    target_link_libraries(PhSpCuda
    ${ROOT_LIBRARIES}
    rt    
    )

endif()


get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

add_executable(PhSp src/PhSp.cpp)

set(CMAKE_CXX_FLAGS "-D_GLIBCXX_USE_CXX11_ABI=0 -DTHRUST_VARIADIC_TUPLE --std=c++11  -march=native -fPIC -O3 -fopenmp -lgomp -Wl,--no-undefined,--no-allow-shlib-undefined" CACHE STRING "compile flags" FORCE)

set_target_properties(PhSp PROPERTIES 
COMPILE_FLAGS "-fopenmp -DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_CPP -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP -lgomp")

if(ROOT_FOUND)
target_link_libraries(PhSp
${ROOT_LIBRARIES}
)
endif()

