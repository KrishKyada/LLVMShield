cmake_minimum_required(VERSION 3.10)
project(WarpAAI_ObfuscationPass)

# Set C++ standard to C++14 (compatible with LLVM 10-14)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM installation
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# LLVM configuration
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Find required LLVM components
llvm_map_components_to_libnames(llvm_libs 
    Support 
    Core 
    IRReader 
    Analysis
    Passes
    TransformUtils
    ScalarOpts
)

# Compiler-specific flags
if(MSVC)
    # MSVC: Disable RTTI and exceptions to match LLVM build
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /EHs-c-")
    # Disable common warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4146 /wd4244 /wd4267 /wd4800")
else()
    # GCC/Clang: Disable RTTI and exceptions, enable warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    # Enable position independent code for shared library
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# Create the shared library (LLVM pass plugin)
add_library(SimpleObfPass SHARED
    SimpleObfPass.cpp
)

# Link against LLVM libraries
target_link_libraries(SimpleObfPass ${llvm_libs})

# Set library properties
set_target_properties(SimpleObfPass PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Platform-specific library naming
if(WIN32)
    # Windows: SimpleObfPass.dll
    set_target_properties(SimpleObfPass PROPERTIES
        SUFFIX ".dll"
        PREFIX ""
    )
elseif(APPLE)
    # macOS: libSimpleObfPass.dylib
    set_target_properties(SimpleObfPass PROPERTIES
        PREFIX "lib"
        SUFFIX ".dylib"
    )
else()
    # Linux: libSimpleObfPass.so
    set_target_properties(SimpleObfPass PROPERTIES
        PREFIX "lib"
        SUFFIX ".so"
    )
endif()

# Print helpful configuration information
message(STATUS "")
message(STATUS "warp_aai Configuration Summary:")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  LLVM Version: ${LLVM_PACKAGE_VERSION}")
message(STATUS "  LLVM Include Dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "  LLVM Libraries: ${llvm_libs}")
message(STATUS "  Plugin Output: ${CMAKE_BINARY_DIR}/lib/")
message(STATUS "")

# Add a custom target to display usage instructions
add_custom_target(usage
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "warp_aai Build Complete!"
    COMMAND ${CMAKE_COMMAND} -E echo "========================="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Plugin built at: ${CMAKE_BINARY_DIR}/lib/"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Usage example:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ./warp_aai.py example.c --pass-lib ${CMAKE_BINARY_DIR}/lib/$<TARGET_FILE_NAME:SimpleObfPass>"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMENT "Display usage instructions"
)

# Optional: Add install rules for deployment
install(TARGETS SimpleObfPass
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
)

install(FILES README.md
    DESTINATION .
)

# Add a simple test to verify plugin can be loaded
enable_testing()
add_test(NAME plugin_load_test
    COMMAND opt -load ${CMAKE_BINARY_DIR}/lib/$<TARGET_FILE_NAME:SimpleObfPass> -help
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)