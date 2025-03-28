# Find dependencies
find_package(AudioFile CONFIG)
find_package(gpu_primitives CONFIG)
find_package(os_utilities CONFIG)
find_package(processor_api CONFIG)
find_package(processor_utilities CONFIG)
find_package(GTest CONFIG)
if(APPLE)
    find_package(metal-cpp CONFIG)
else()
    find_package(hip_sdk CONFIG)

    #For additional CUDA targets and PATHS
    #https://cmake.org/cmake/help/latest/module/FindCUDAToolkit.html
    find_package(CUDAToolkit ${CUDA_VERSION})

    set(CMAKE_CUDA_SEPARABLE_COMPILATION OFF)
    set(CMAKE_CUDA_RUNTIME_LIBRARY Static)
    set(CMAKE_CUDA_FLAGS "--relocatable-device-code=true -maxrregcount=${CUDA_MAXRREGCOUNT}")
endif()

# Component name.
set(component_id ${effect_name})
BG_FirstCaseUpper(component_id_capitalized "${component_id}")
string(TOUPPER "${component_id}" component_id_uppercase)
set(component_name ${component_id}_processor)

# Component type (library or executable).
set(component_type library)

if(APPLE)
    # Library type (STATIC, SHARED, MODULE, INTERFACE).
    set(library_type MODULE)
else()
    # Library type (STATIC, SHARED, MODULE, INTERFACE).
    set(library_type INTERFACE)

    set(nvidia_library_type MODULE)
    set(amd_library_type MODULE)
endif()

# target libraries
if(LINUX)
    set(linux_common_private_target_libraries
        tbb
    )
endif()

set(common_private_target_libraries
    AudioFile::AudioFile
    gpu_primitives::gpu_primitives
    os_utilities::os_utilities
    processor_api::processor_api
    processor_utilities::processor_utilities
    ${linux_common_private_target_libraries}
)

set(device_common_private_target_libraries
    gpu_primitives::gpu_primitives
    processor_utilities::processor_utilities
)

if(APPLE)
    set(private_target_libraries
        ${common_private_target_libraries}
        metal-cpp::metal-cpp
    )

    set(device_metal_private_target_libraries
        ${device_common_private_target_libraries}
    )
else()

    set(nvidia_private_target_libraries
        ${common_private_target_libraries}
        CUDA::cudart_static
    )

    set(amd_private_target_libraries
        ${common_private_target_libraries}
    )

    set(device_nvidia_private_target_libraries
        ${device_common_private_target_libraries}
    )

    set(device_amd_private_target_libraries
        ${device_common_private_target_libraries}
    )
endif()

set(public_target_libraries
)

set(interface_target_libraries
)

# compile definitions
if(WIN32)
    set(win_common_private_compile_definitions
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        WIN32
        WIN64
    )
endif()

set(module_common_private_compile_definitions
    MODULE_EFFECT_NAME="${component_id_uppercase}"
    MODULE_ID="${component_id}"
    MODULE_MAJOR_VERSION=${${component_name}_MAJOR_VERSION}
    MODULE_MINOR_VERSION=${${component_name}_MINOR_VERSION}
    MODULE_PATCH_LEVEL=${${component_name}_PATCH_LEVEL}
)

set(common_private_compile_definitions
    ${win_common_private_compile_definitions}
    MODULE_IMPLEMENTATION
    ${module_common_private_compile_definitions}
)

if(APPLE)
    set(private_compile_definitions
        GPU_AUDIO_MAC # TODO: this should be handled by bg-conan-common
        ${common_private_compile_definitions}
    )
else()
    set(nvidia_private_compile_definitions
        ${common_private_compile_definitions}
    )

    set(amd_private_compile_definitions
        ${common_private_compile_definitions}
    )
endif()

if(NOT APPLE)
    # static_assert(false, "GPU profiling not implemented for mac\n"); in gpu_utilities
    set(common_device_private_compile_definitions
        $<$<CONFIG:Release>:N>GPU_PROFILING
    )
endif()

if(APPLE)
    set(device_metal_private_compile_definitions
        ${common_device_private_compile_definitions}
    )
else()
    set(device_nvidia_private_compile_definitions
        ${common_device_private_compile_definitions}
    )

    set(device_amd_private_compile_definitions
        ${common_device_private_compile_definitions}
    )
endif()

set(public_compile_definitions
)

set(interface_compile_definitions
)

# compile options
set(private_compile_options
)

set(public_compile_options
)

set(interface_compile_options
)

# link options
set(private_link_options
)

set(public_link_options
)

set(interface_link_options
)

# List of private include directories.
set(common_include_directories
    src/convolution_filter
    src/device
)
if(APPLE)
    set(private_include_directories
        ${common_include_directories}
    )

    set(device_metal_private_include_directories
        src/device
    )
else()
    set(nvidia_private_include_directories
        ${common_include_directories}
    )

    set(amd_private_include_directories
        ${common_include_directories}
    )

    set(device_nvidia_private_include_directories
        src/device
    )

    set(device_amd_private_include_directories
        src/device
    )
endif()

# List of public include directories.
set(common_public_include_directories
    include
)
if(APPLE)
    set(public_include_directories
        ${common_public_include_directories}
    )
else()
    set(nvidia_public_include_directories
        ${common_public_include_directories}
    )

    set(amd_public_include_directories
        ${common_public_include_directories}
    )
endif()

# List of private header files.
set(common_private_headers
    include/fir_processor/FirSpecification.h
    src/${component_id_capitalized}DeviceCodeProvider.h
    src/${component_id_capitalized}Module.h
    src/${component_id_capitalized}ModuleInfoProvider.h
    src/${component_id_capitalized}Processor.h
    src/convolution_filter/ConvolutionFilter.h
    src/convolution_filter/IRFilter.h
    src/convolution_filter/StaticIRShare.h
    src/ImpulseResponseStore.h
)

if(APPLE)
    set(private_headers
        ${common_private_headers}
    )

    set(device_metal_private_headers
        src/device/${component_id_capitalized}Processor.cuh
        src/device/Properties.h
        src/device/SM_FFT_parameters.cuh
    )
else()
    set(nvidia_private_headers
        ${common_private_headers}
    )

    set(amd_private_headers
        ${common_private_headers}
    )

    set(device_nvidia_private_headers
        src/device/${component_id_capitalized}Processor.cuh
        src/device/Properties.h
        src/device/SM_FFT_parameters.cuh
    )

    set(device_amd_private_headers
        src/device/${component_id_capitalized}Processor.cuh
        src/device/Properties.h
        src/device/SM_FFT_parameters.cuh
    )
endif()

# List of source files.
set(common_sources
    src/${component_id_capitalized}DeviceCodeProvider.cpp
    src/${component_id_capitalized}Module.cpp
    src/${component_id_capitalized}ModuleInfoProvider.cpp
    src/${component_id_capitalized}ModuleLibrary.cpp
    src/${component_id_capitalized}Processor.cpp
    src/convolution_filter/IRFilter.cpp
    src/convolution_filter/StaticIRShare.cpp
    src/ImpulseResponseStore.cpp

)

if(APPLE)
    set(sources
        ${common_sources}
    )

    set(device_metal_sources
        src/device/${component_id_capitalized}Processor.cu
    )
else()
    set(nvidia_sources
        ${common_sources}
    )

    set(amd_sources
        ${common_sources}
    )

    set(device_nvidia_sources
        src/device/${component_id_capitalized}Processor.cu
    )

    set(device_amd_sources
        src/device/${component_id_capitalized}Processor.cu
    )
endif()

# List of test source files.
set(common_test_private_compile_definitions
    ${win_common_private_compile_definitions}
)

if(APPLE)
    set(metal_test_private_compile_definitions
        ${common_test_private_compile_definitions}
    )
else()
    set(nvidia_test_private_compile_definitions
        ${common_test_private_compile_definitions}
    )

    set(amd_test_private_compile_definitions
        ${common_test_private_compile_definitions}
    )
endif()

set(common_test_headers
    tests/TestCommon.h
)

if(APPLE)
    set(metal_test_headers
        ${common_test_headers}
    )
else()
    set(nvidia_test_headers
        ${common_test_headers}
    )

    set(amd_test_headers
        ${common_test_headers}
    )
endif()

set(common_test_sources
    tests/${component_id_capitalized}ModuleInfoProviderTests.cpp
)

if(APPLE)
    set(metal_test_sources
        ${common_test_sources}
    )
else()
    set(nvidia_test_sources
        ${common_test_sources}
    )

    set(amd_test_sources
        ${common_test_sources}
    )
endif()

if(NOT APPLE)
    # TODO: Fix parallel test execution for AMD
    set(amd_test_properties
        RUN_SERIAL TRUE
    )
endif()

# Link a test target to the given libraries.
if(LINUX)
    set(linux_common_test_private_target_libraries
        ${CMAKE_DL_LIBS}
    )
endif()

set(common_test_private_target_libraries
    GTest::gtest_main
    os_utilities::os_utilities
    processor_api::processor_api
    processor_utilities::processor_utilities
    ${linux_common_test_private_target_libraries}
)

if(APPLE)
    set(metal_test_private_target_libraries
        ${common_test_private_target_libraries}
    )
else()
    set(nvidia_test_private_target_libraries
        ${common_test_private_target_libraries}
    )

    set(amd_test_private_target_libraries
        ${common_test_private_target_libraries}
    )
endif()

# Set tests build dependencies.
if(APPLE)
    set(metal_test_target_dependencies
        ${component_name}
    )
else()
    set(nvidia_test_target_dependencies
        ${component_name}_nvidia
    )

    set(amd_test_target_dependencies
        ${component_name}_amd
    )
endif()
