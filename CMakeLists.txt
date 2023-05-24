cmake_minimum_required(VERSION 3.8)
project(path-renderer)

add_subdirectory(ext ext_build)

add_executable(path_renderer
        src/main.cpp

        src/core/common.h
        src/core/common.cpp
        src/core/parser.h
        src/core/parser.cpp
        src/core/object.h
        src/core/object.cpp
        )

target_include_directories(path_renderer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${EIGEN_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${TBB_INCLUDE_DIR}
        ${OPENEXR_INCLUDE_DIRS}
        ${PUGIXML_INCLUDE_DIR}
        )

get_property(dirs TARGET path_renderer PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()

target_link_libraries(path_renderer PUBLIC tbb pugixml nanogui eigen ${NANOGUI_EXTRA_LIBS} OpenEXR::OpenEXR)
target_compile_features(path_renderer PRIVATE cxx_std_17)