//
// Created by juperez on 5/26/23.
//

#pragma once

#include "core/color.h"
#include "primitives/vector.h"

LUMINA_NAMESPACE_BEGIN

class Bitmap : public Eigen::Array<Color3f, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> {
public:
    typedef Eigen::Array<Color3f, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Base;

    Bitmap(const Vector2i& size = Vector2i(0, 0))
        : Base(size.x(), size.y()) {}

    Bitmap(const std::string& filename);

    void saveEXR(const std::string& filename);
    void savePNG(const std::string& filename);
};

LUMINA_NAMESPACE_END