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

Vector3f Warp::squareToTriangle(const Point2f &sample) {
    float root = std::sqrt(1 - sample.x());
    float b1 = 1.0f - root, b2 = sample.y() * root;

    return Vector3f(b1, b2, 1 - b2 - b1);
}

float Warp::squareToTrianglePdf(const Vector3f &v) {
    return 0;
}

LUMINA_NAMESPACE_END