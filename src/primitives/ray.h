//
// Created by juperez on 5/24/23.
//

#pragma once

#include "vector.h"

LUMINA_NAMESPACE_BEGIN

template <typename PointType, typename VectorType> struct TRay {
    typedef typename PointType::Scalar  Scalar;

    PointType o;     ///< Ray origin
    VectorType d;    ///< Ray direction
    VectorType dRcp; ///< Componentwise reciprocals of the ray direction
    Scalar mint;     ///< Minimum position on the ray segment
    Scalar maxt;     ///< Maximum position on the ray segment

    /// Construct a new ray
    TRay() : mint(Epsilon),
             maxt(std::numeric_limits<Scalar>::infinity()) { }

    /// Construct a new ray
    TRay(const PointType &o, const VectorType &d) : o(o), d(d),
                                                    mint(Epsilon), maxt(std::numeric_limits<Scalar>::infinity()) {
        update();
    }

    /// Construct a new ray
    TRay(const PointType &o, const VectorType &d,
         Scalar mint, Scalar maxt) : o(o), d(d), mint(mint), maxt(maxt) {
        update();
    }

    /// Copy constructor
    TRay(const TRay &ray)
            : o(ray.o), d(ray.d), dRcp(ray.dRcp),
              mint(ray.mint), maxt(ray.maxt) { }

    /// Copy a ray, but change the covered segment of the copy
    TRay(const TRay &ray, Scalar mint, Scalar maxt)
            : o(ray.o), d(ray.d), dRcp(ray.dRcp), mint(mint), maxt(maxt) { }

    /// Update the reciprocal ray directions after changing 'd'
    void update() {
        dRcp = d.cwiseInverse();
    }

    /// Return the position of a point along the ray
    PointType operator() (Scalar t) const { return o + t * d; }

    /// Return a ray that points into the opposite direction
    TRay reverse() const {
        TRay result;
        result.o = o; result.d = -d; result.dRcp = -dRcp;
        result.mint = mint; result.maxt = maxt;
        return result;
    }

    /// Return a human-readable string summary of this ray
    std::string toString() const {
        return tfm::format(
                "Ray[\n"
                "  o = %s,\n"
                "  d = %s,\n"
                "  mint = %f,\n"
                "  maxt = %f\n"
                "]", o.toString(), d.toString(), mint, maxt);
    }
};
LUMINA_NAMESPACE_END
