cmake_minimum_required(VERSION 3.20)
project(screen_capture_fr)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)
add_executable(screen_capture_fr
        main/ScreenCapture.cpp main/ScreenCapture.h main/main.cpp)



find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavcodec
        libavdevice
        libavfilter
        libavformat
        libavutil
        libswscale
        libswresample
        )
target_link_libraries(${PROJECT_NAME}
        PkgConfig::LIBAV
        pthread
        )

