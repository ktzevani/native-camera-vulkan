cmake_minimum_required(VERSION 3.19.0-rc1)

project(native-camera-vulkan)

set(BUILD_FLAVOR
        #"SIMPLE_VULKAN"
        "COMPLEX_VULKAN"
)

if(BUILD_FLAVOR STREQUAL "SIMPLE_VULKAN")
        add_definitions(-DNCV_USE_VULKAN_SIMPLE)
elseif(BUILD_FLAVOR STREQUAL "COMPLEX_VULKAN")
        add_definitions(-DNCV_USE_VULKAN_COMPLETE)
endif()

add_definitions(
        -DVK_USE_PLATFORM_ANDROID_KHR
        -DVK_NO_PROTOTYPES
        -DNDEBUG
        #-DNCV_VULKAN_VALIDATION_ENABLED
        #-DNCV_LOGGING_ENABLED
        #-DNCV_PROFILING_ENABLED
)

add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

set(Project_SOURCES core/*.cpp devices/*.cpp engine/*.cpp vk_util/*.cpp main.cpp)

if(BUILD_FLAVOR STREQUAL "SIMPLE_VULKAN")
        set(Project_SOURCES ${Project_SOURCES}
                graphics/simple_context.cpp
                graphics/vulkan_context.cpp)
elseif(BUILD_FLAVOR STREQUAL "COMPLEX_VULKAN")
        set(Project_SOURCES ${Project_SOURCES}
                graphics/data/*.cpp
                graphics/resources/*.cpp
                graphics/pipeline.cpp
                graphics/complex_context.cpp
                graphics/vulkan_context.cpp)
endif()

file(GLOB SOURCES ${Project_SOURCES})

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/metadata/version.cpp
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake
        DEPENDS ${SOURCES}
        COMMENT "Updating Version"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(app_version SHARED ${CMAKE_CURRENT_SOURCE_DIR}/metadata/version.cpp)
add_library(native-camera-vulkan SHARED ${SOURCES})

include_directories(${ANDROID_NDK}/sources/android/native_app_glue
        $ENV{LIBRARIES_ROOT}/vulkan
        $ENV{LIBRARIES_ROOT}/vulkan_hpp
        $ENV{LIBRARIES_ROOT}/boost
        $ENV{LIBRARIES_ROOT}/stb
        $ENV{LIBRARIES_ROOT}/glm
        .)

target_link_libraries(native-camera-vulkan
        android
        dl
        native_app_glue
        app_version
        log
        mediandk
        camera2ndk)