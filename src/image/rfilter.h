//
// Created by juperez on 5/26/23.
//

#pragma once

#include "core/object.h"

#define LUMINA_FILTER_RESOLUTION 32

LUMINA_NAMESPACE_BEGIN

class ReconstructionFilter : public LuminaObject {
public:
    float getRadius() const { return m_radius; }

    virtual float eval(float x) const = 0;

    EClassType getClassType() const { return EReconstructionFilter; }

protected:
    float m_radius;
};

LUMINA_NAMESPACE_END