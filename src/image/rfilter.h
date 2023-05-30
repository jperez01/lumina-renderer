//
// Created by juperez on 5/26/23.
//

#pragma once

#include "core/object.h"

#define LUMINA_FILTER_RESOLUTION 32

LUMINA_NAMESPACE_BEGIN

/**
 * \brief Generic radially symmetric image reconstruction filter
 *
 * When adding radiance-valued samples to the rendered image, Nori
 * first convolves them with a so-called image reconstruction filter.
 *
 * To learn more about reconstruction filters and sampling theory
 * in general, take a look at the excellenent chapter 7 of PBRT,
 * which is freely available at:
 *
 * http://graphics.stanford.edu/~mmp/chapters/pbrt_chapter7.pdf
 */
class ReconstructionFilter : public LuminaObject {
public:
    /// Return the filter radius in fractional pixels
    float getRadius() const { return m_radius; }

    /// Evaluate the filter function
    virtual float eval(float x) const = 0;

    /**
     * \brief Return the type of object (i.e. Mesh/Camera/etc.)
     * provided by this instance
     * */
    EClassType getClassType() const { return EReconstructionFilter; }
protected:
    float m_radius;
};

LUMINA_NAMESPACE_END