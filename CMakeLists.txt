cmake_minimum_required(VERSION 3.8)
project(path-renderer)

add_subdirectory(ext ext_build)

add_executable(path_renderer
        src/main.cpp

        src/core/common.h
        src/core/common.cpp
        src/core/object.h
        src/core/object.cpp
        src/core/propsList.h
        src/core/propsList.cpp
        src/core/color.h
        src/core/color.cpp

        src/primitives/transform.h
        src/primitives/vector.h
        src/primitives/ray.h

        src/utils/parser.h
        src/utils/parser.cpp
        src/utils/warp.h
        src/utils/warp.cpp
        src/utils/resolver.h
        src/utils/timer.h
        src/utils/sampler.h
        src/utils/sampler.cpp
        src/utils/dpdf.h

        src/scene/camera.h
        src/scene/camera.cpp
        src/scene/scene.h
        src/scene/scene.cpp
        src/scene/accel.h
        src/scene/accel.cpp

        src/primitives/frame.h
        src/primitives/mesh.h
        src/primitives/mesh.cpp
        src/primitives/bbox.h
        src/primitives/objMesh.h
        src/primitives/objMesh.cpp

        src/lights/emitter.h
        src/lights/areaLight.h
        src/lights/areaLight.cpp

        src/bsdfs/bsdf.h
        src/bsdfs/diffuse.cpp
        src/bsdfs/dielectric.cpp
        src/bsdfs/mirror.cpp

        src/image/block.h
        src/image/block.cpp
        src/image/rfilter.h
        src/image/bitmap.h
        src/image/bitmap.cpp
        src/image/gui.h
        src/image/gui.cpp
        src/image/rfilter.cpp

        src/integrators/integrator.h
        src/integrators/normals.cpp
        src/integrators/pathEms.cpp
        src/integrators/pathMis.cpp
        src/integrators/distributed.cpp
        src/integrators/pathMats.cpp
        )

target_include_directories(path_renderer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${EIGEN_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${TBB_INCLUDE_DIR}
        ${OPENEXR_INCLUDE_DIRS}
        ${PUGIXML_INCLUDE_DIR}
        ${TINY_INCLUDE_DIR}
        )

get_property(dirs TARGET path_renderer PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()

target_link_libraries(path_renderer PUBLIC tbb pcg32 tinyformat pugixml nanogui eigen ${NANOGUI_EXTRA_LIBS} OpenEXR::OpenEXR)
target_compile_features(path_renderer PRIVATE cxx_std_17)