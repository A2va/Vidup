# Set CMake version
cmake_minimum_required(VERSION 3.19.3)

# Name 
project(Vidup)
set(VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


# Set build and install path
set(CMAKE_BUILD_DIRECTORY build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY output)
set(CMAKE_INSTALL_FOLDER ${CMAKE_SOURCE_DIR}/install)
set(CMAKE_SOURCE_DIR app)

include_directories(${CMAKE_SOURCE_DIR})


set(USE_STATIC_LINKING ON)
# Set compiler flags
set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS ${CMAKE_CXX_FLAGS}")

if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE OFF)
endif()

set(SOURCES
      app/main.cpp
      app/ffmpeg_tests.cpp
      app/ffmpeg_tests.h

      app/mainwindow.cpp
      app/mainwindow.h
      app/mainwindow.ui

      app/ffmpeg/codecparam.cpp
      app/ffmpeg/codecparam.h
      app/ffmpeg/ffmpegerror.cpp
      app/ffmpeg/ffmpegerror.h
      app/ffmpeg/ffmpegencoder.cpp
      app/ffmpeg/ffmpegencoder.h
      app/ffmpeg/ffmpegdecoder.cpp
      app/ffmpeg/ffmpegdecoder.h
)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package( OpenCV REQUIRED )
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)

include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} )
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} PkgConfig::LIBAV)
