cmake_minimum_required(VERSION 3.10.2)

project("cxrnativesdk")

# Import CloudXR
add_library(cloudxr-lib SHARED IMPORTED)
set_target_properties(cloudxr-lib PROPERTIES IMPORTED_LOCATION
        ${CLOUDXR_LIBPATH}/${ANDROID_ABI}/libCloudXRClient.so
        )

include_directories(${CLOUDXR_INCLUDE})

add_library( # Sets the name of the library.
        native-lib
        SHARED
        native-lib.cpp
        CloudXRClient.cpp)

find_package (oboe
        REQUIRED
        CONFIG)

target_link_libraries( # Specifies the target library.
        native-lib
        cloudxr-lib
        android
        log
        GLESv3
        EGL
        oboe::oboe)