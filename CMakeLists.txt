cmake_minimum_required(VERSION 3.8)
project(path-renderer)

add_subdirectory(ext ext_build)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        SYSTEM ${EIGEN_INCLUDE_DIR}
        SYSTEM ${GLFW_INCLUDE_DIR}
        SYSTEM ${GLEW_INCLUDE_DIR}
        SYSTEM ${NANOGUI_INCLUDE_DIR}
        SYSTEM ${TBB_INCLUDE_DIR}
        SYSTEM ${OPENEXR_INCLUDE_DIRS}
)

add_executable(path_renderer
        src/main.cpp)

target_link_libraries(path_renderer tbb nanogui ${NANOGUI_EXTRA_LIBS} OpenEXR::OpenEXR)
target_compile_features(path_renderer PRIVATE cxx_std_17)