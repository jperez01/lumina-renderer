# Build Thread Building Blocks Library
add_subdirectory(tbb)
set_property(TARGET tbb PROPERTY FOLDER "dependencies")

# Build NanoGUI Library
set(NANOGUI_BUILD_SHARED OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON OFF CACHE BOOL " " FORCE)
add_subdirectory(nanogui)
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")

# Build OpenEXR
add_subdirectory(openexr)

# Build Eigen
add_subdirectory(eigen)

set(GLFW_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/glfw/include)
set(GLEW_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/glew/include)
set(NANOVG_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/nanovg/src)
set(STB_IMAGE_WRITE_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/nanovg/example)
set(NANOGUI_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/include)
set(EIGEN_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/eigen)
set(TBB_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/tbb/include)