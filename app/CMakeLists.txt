# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/thirdparts/include)
link_directories(${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a)
#set(distribution_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})


#支持-std=gnu++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

add_library(avutil
            SHARED
            IMPORTED )
set_target_properties(
    avutil
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}//src/main/jniLibs/armeabi-v7a/libavutil-55.so
    )

 add_library(swresample
             SHARED
             IMPORTED )
 set_target_properties(
     swresample
     PROPERTIES IMPORTED_LOCATION
     ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libswresample-2.so
     )

add_definitions(-DUSE_ARM_NEON)

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp
             src/main/cpp/AudioResample.cpp
             src/main/cpp/CCycleBuffer.cpp
             src/main/cpp/AILog.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib
                       swresample
                       avutil
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib})