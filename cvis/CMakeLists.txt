cmake_minimum_required(VERSION 3.7...3.18)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
	cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(cvis LANGUAGES CXX C)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
	libavdevice
	libavfilter
	libavformat
	libavcodec
	libswresample
	libswscale
	libavutil)

add_executable(${PROJECT_NAME}
	src/analyze.cpp
	src/codec/decoding.cpp
	src/codec/encoding.cpp
	src/deps/stb.cpp
	src/main.cpp
	src/tetrik/tetrik.cpp
	src/util.cpp
	lib/fmt/src/format.cc
	lib/glad/src/glad.c)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

target_include_directories(${PROJECT_NAME}
	PUBLIC src
	PUBLIC lib/fmt/include
	PUBLIC lib/glad/include
	PUBLIC lib/glm
	PUBLIC lib/stb)

target_link_libraries(${PROJECT_NAME} OpenGL::GL)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} PkgConfig::LIBAV)
target_link_libraries(${PROJECT_NAME} fftw3)

add_custom_target(shaders COMMAND cp -R ${CMAKE_SOURCE_DIR}/src/shaders ${CMAKE_BINARY_DIR})
add_dependencies(cvis shaders)

add_custom_target(tetrik COMMAND ${PROJECT_NAME} --endframe ../media/endframe.png 452.684 --silence 0.75 ../media/tetrik.flac ../media/tetrik.mp4)
add_dependencies(tetrik cvis)
