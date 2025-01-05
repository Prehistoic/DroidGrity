
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

add_library(
        ${CMAKE_PROJECT_NAME} 
        
        SHARED
        
        droidgrity.cpp
        mylibc.cpp
        path_helper.cpp
)

target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        
        # List libraries link to the target library
        android
        log
)
