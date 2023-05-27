//
// Created by juperez on 5/24/23.
//

#pragma once

#include "core/common.h"
#include "vector.h"
#include "primitives/ray.h"

LUMINA_NAMESPACE_BEGIN

struct Transform {
public:
    Transform() :
            m_transform(Eigen::Matrix4f::Identity()),
            m_inverse(Eigen::Matrix4f::Identity()) { }

    Transform(const Eigen::Matrix4f &trafo);

    Transform(const Eigen::Matrix4f &trafo, const Eigen::Matrix4f &inv)
            : m_transform(trafo), m_inverse(inv) { }

    const Eigen::Matrix4f &getMatrix() const {
        return m_transform;
    }

    const Eigen::Matrix4f &getInverseMatrix() const {
        return m_inverse;
    }

    Transform inverse() const {
        return Transform(m_inverse, m_transform);
    }

    Transform operator*(const Transform &t) const;

    Vector3f operator*(const Vector3f &v) const {
        return m_transform.topLeftCorner<3,3>() * v;
    }

    Normal3f operator*(const Normal3f &n) const {
        return m_inverse.topLeftCorner<3, 3>().transpose() * n;
    }

    Point3f operator*(const Point3f &p) const {
        Vector4f result = m_transform * Vector4f(p[0], p[1], p[2], 1.0f);
        return result.head<3>() / result.w();
    }

    Ray3f operator*(const Ray3f &r) const {
        return Ray3f(
                operator*(r.o),
                operator*(r.d),
                r.mint, r.maxt
        );
    }

    /// Return a string representation
    std::string toString() const;
private:
    Eigen::Matrix4f m_transform;
    Eigen::Matrix4f m_inverse;
};

LUMINA_NAMESPACE_END