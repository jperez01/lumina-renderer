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

Vector3f Warp::squareToBeckmann(const Point2f& sample, float alpha) {
    float x, y, z, phi, logSample, cos_theta, sin_theta, tan2theta;

    phi = 2 * M_PI * sample[0];
    logSample = std::log(std::max((float)0, 1 - sample[1]));
    // logSample can be a number from -inf to 0.
    tan2theta = -alpha * alpha * logSample;
    // tan2theta can be a number from inf to 0.
    cos_theta = (float)1 / std::sqrt(1 + tan2theta);
    // cos_theta can be a number from 0 to 1.
    sin_theta = std::sqrt(std::max((float)0, (float)1. - cos_theta * cos_theta));

    // Now from polar to cartesian
    x = sin_theta * std::cos(phi);
    y = sin_theta * std::sin(phi);
    z = cos_theta;

    return Vector3f(x, y, z);
}

float Warp::squareToBeckmannPdf(const Vector3f& v, float alpha) {
    float tan2theta, cos_theta, alpha2, prob, denom;
    alpha2 = alpha * alpha;
    // From z we have cos theta and tan
    cos_theta = v[2];

    if (cos_theta == 0) {
        prob = 0;
    }
    else {
        tan2theta = ((float)1. / (cos_theta * cos_theta)) - 1;
        denom = M_PI * alpha2 * cos_theta * cos_theta * cos_theta;
        prob = std::exp(-tan2theta / alpha2) / denom;
    }

    return (v[2] >= 0 && (std::abs(std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]) - 1) < 0.0001)) ? prob : 0.0f;
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