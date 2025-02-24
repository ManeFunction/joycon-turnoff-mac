cmake_minimum_required(VERSION 3.10)
project(jcoff)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    # Detect architecture
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Set Homebrew prefix based on architecture
    if(ARCH STREQUAL "arm64")
        set(BREW_PREFIX "/opt/homebrew")
    else()
        set(BREW_PREFIX "/usr/local")
    endif()

    find_library(HIDAPI_LIBRARY 
        NAMES hidapi
        PATHS "${BREW_PREFIX}/lib"
        NO_DEFAULT_PATH
        REQUIRED
    )
    
    find_path(HIDAPI_INCLUDE_DIR 
        NAMES hidapi.h
        PATHS "${BREW_PREFIX}/include"
        PATH_SUFFIXES hidapi
        NO_DEFAULT_PATH
        REQUIRED
    )
    
    set(MAC_FRAMEWORKS
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework AppKit"
    )
else()
    # Linux dynamic build
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(HIDAPI REQUIRED hidapi-hidraw)
    pkg_check_modules(UDEV REQUIRED libudev)
    pkg_check_modules(USB REQUIRED libusb-1.0)
endif()

add_executable(jcoff jcoff/jcoff_port.cpp)

if(APPLE)
    target_include_directories(jcoff PRIVATE ${HIDAPI_INCLUDE_DIR})
    target_link_libraries(jcoff PRIVATE ${HIDAPI_LIBRARY} ${MAC_FRAMEWORKS})
else()
    target_include_directories(jcoff PRIVATE ${HIDAPI_INCLUDE_DIRS})
    target_link_libraries(jcoff PRIVATE ${HIDAPI_LIBRARIES} ${UDEV_LIBRARIES} ${USB_LIBRARIES})
endif() 