//
// Created by juperez on 5/25/23.
//

#pragma once

#include "core/common.h"

LUMINA_NAMESPACE_BEGIN

namespace Warp {
    Vector3f squareToCosineHemisphere(const Point2f& sample);
    float squareToCosineHemispherePdf(const Vector3f& v);

    Vector3f squareToTriangle(const Point2f& sample);
    float squareToTrianglePdf(const Vector3f& v);
};

LUMINA_NAMESPACE_END
