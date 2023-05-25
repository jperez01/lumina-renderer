# Build Thread Building Blocks Library
add_subdirectory(tbb)
set_property(TARGET tbb PROPERTY FOLDER "dependencies")

# Build NanoGUI Library
set(NANOGUI_BUILD_SHARED OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON OFF CACHE BOOL " " FORCE)
add_subdirectory(nanogui)
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")

# Build OpenEXR
set(ILMBASE_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
set(OPENEXR_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
set(ILMBASE_NAMESPACE_VERSIONING OFF CACHE BOOL " " FORCE)
set(OPENEXR_NAMESPACE_VERSIONING OFF CACHE BOOL " " FORCE)
set(OPENEXR_FORCE_INTERNAL_IMATH ON CACHE BOOL " " FORCE)
add_subdirectory(openexr)

# Build Eigen
add_subdirectory(eigen)
set_property(TARGET eigen PROPERTY FOLDER "dependencies")

# Build pugixml
add_subdirectory(pugixml)
set_property(TARGET pugixml PROPERTY FOLDER "dependencies")

add_library(tinyformat INTERFACE tinyformat/tinyformat.h)
target_include_directories(tinyformat INTERFACE .)

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
set(PUGIXML_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/pugixml/src)
set(TINY_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/tinyformat)

set(CompilerFlags
        GLFW_INCLUDE_DIR GLEW_INCLUDE_DIR NANOVG_INCLUDE_DIR STB_IMAGE_WRITE_INCLUDE_DIR
        NANOGUI_INCLUDE_DIR EIGEN_INCLUDE_DIR TBB_INCLUDE_DIR PUGIXML_INCLUDE_DIR)

foreach(CompilerFlag ${CompilerFlags})
    message("${${CompilerFlag}}")
    set(${CompilerFlag} "${${CompilerFlag}}" PARENT_SCOPE)
endforeach()