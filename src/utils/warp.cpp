//
// Created by juperez on 5/25/23.
//

#include "warp.h"
#include "primitives/vector.h"

LUMINA_NAMESPACE_BEGIN

Vector3f Warp::squareToCosineHemisphere(const Point2f& sample) {
    float phi = acos(sqrt(sample.x())), theta = 2 * M_PI * sample.y();

    return Vector3f(
            sin(phi) * cos(theta),
            sin(phi) * sin(theta),
            cos(phi)
            );
}

float Warp::squareToCosineHemispherePdf(const Vector3f &v) {
    if (v.x() * v.x() + v.y() * v.y() + v.z() * v.z() <= 1.0f && v.z() >= 0)
        return v.z() * INV_PI;
    else
        return 0.0f;
}

LUMINA_NAMESPACE_END