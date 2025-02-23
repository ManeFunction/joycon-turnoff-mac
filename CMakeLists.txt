cmake_minimum_required(VERSION 3.10)
project(jcoff)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On macOS, we use hidapi-mac from Homebrew
if(APPLE)
    # Find Homebrew's hidapi location
    execute_process(
        COMMAND brew --prefix hidapi
        OUTPUT_VARIABLE HIDAPI_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    set(HIDAPI_INCLUDE_DIR "${HIDAPI_PREFIX}/include")
    set(HIDAPI_LIBRARY "${HIDAPI_PREFIX}/lib/libhidapi.dylib")
else()
    # For other platforms, use pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(HIDAPI REQUIRED hidapi)
endif()

# Add source files
add_executable(jcoff
    jcoff/jcoff_mac.cpp
)

# Include directories
target_include_directories(jcoff PRIVATE
    ${CMAKE_SOURCE_DIR}/jcoff
    ${HIDAPI_INCLUDE_DIR}
)

# Link libraries
if(APPLE)
    target_link_libraries(jcoff PRIVATE ${HIDAPI_LIBRARY})
else()
    target_link_libraries(jcoff PRIVATE ${HIDAPI_LIBRARIES})
endif() 