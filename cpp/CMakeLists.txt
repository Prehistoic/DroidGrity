
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("droidgrity")

# Adding a build type to enable/disable android logs
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DENABLE_LOGS)
endif()

add_library(
        ${CMAKE_PROJECT_NAME} 
        
        SHARED
        
        droidgrity.cpp
        src/mylibc.cpp
        src/helpers/sha256_helper.cpp
        src/helpers/path_helper.cpp
        src/helpers/apksigningblock_helper.cpp
        src/helpers/unzip_helper.cpp
        src/helpers/inflate_helper.cpp
)

target_include_directories(
        ${CMAKE_PROJECT_NAME}

        PRIVATE

        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/utils
        ${CMAKE_SOURCE_DIR}/include/helpers
)

target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        
        # List libraries link to the target library
        android
        log
        z
)
