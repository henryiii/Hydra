# This allows Add Hydra example to work with either a pair of .cpp and .cu
# files, or a single .cu file for all backends!
function(HYDRA_NORMALIZE INVAR OUTNAME)
    if(EXISTS "${INVAR}.cpp")
        set(${OUTNAME} "${INVAR}.cpp" PARENT_SCOPE)
    else()
        set(${OUTNAME} "${INVAR}.cu" PARENT_SCOPE)
        set_source_files_properties("${INVAR}.cu" PROPERTIES LANGUAGE CXX)
    endif()
endfunction()

# Add an example. Give the filename without the extension (.cu or .cpp)
# Builds all available (requested) backends
function(ADD_HYDRA_EXAMPLE target_name)
    set(ADDINGMESSAGE "Adding:")

    hydra_normalize(${target_name} target_norm)

    # CPP
    if(BUILD_CPP_TARGETS)
        set(ADDINGMESSAGE "${ADDINGMESSAGE} ${target_name}_cpp")
        add_executable(${target_name}_cpp ${target_norm})
        target_link_libraries(${target_name}_cpp PUBLIC ROOT_lib tclap_lib Hydra_cpp)
        target_compile_options(${target_name}_cpp PUBLIC -x c++)

    endif()

    # CUDA
    if(BUILD_CUDA_TARGETS)
        set(ADDINGMESSAGE "${ADDINGMESSAGE} ${target_name}_cuda")
        
        # Add the .cu file if one exists
        cuda_add_executable("${target_name}_cuda"
            "${target_name}.cu"    
            OPTIONS -Xcompiler -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_CUDA  -DTHRUST_HOST_SYSTEM=THRUST_HOST_SYSTEM_CPP)

        target_link_libraries("${target_name}_cuda" ROOT_lib tclap_lib Hydra_cuda )
    endif()

    # TBB
    if(BUILD_TBB_TARGETS)
        set(ADDINGMESSAGE "${ADDINGMESSAGE} ${target_name}_tbb")
        add_executable("${target_name}_tbb" ${target_norm})

        target_link_libraries("${target_name}_tbb" PUBLIC ROOT_lib tclap_lib Hydra_tbb)
        target_compile_options(${target_name}_tbb PUBLIC -x c++)
    endif()

    # OMP
    if(BUILD_OMP_TARGETS)
        set(ADDINGMESSAGE "${ADDINGMESSAGE} ${target_name}_omp")
        add_executable(${target_name}_omp ${target_norm})

        target_link_libraries(${target_name}_omp PUBLIC ROOT_lib tclap_lib Hydra_omp)
        target_compile_options(${target_name}_omp PUBLIC -x c++)
    endif()

    message(STATUS ${ADDINGMESSAGE})
endfunction()
