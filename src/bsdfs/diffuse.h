//
// Created by juperez on 5/25/23.
//

#pragma once

#include "bsdf.h"

LUMINA_NAMESPACE_BEGIN

class Diffuse : public BSDF {
public:
    Diffuse(const PropertyList& propsList);
    Color3f sample(BSDFQueryRecord& bRec, const Point2f& sample) const;

    Color3f eval(BSDFQueryRecord& bRec) const;

    float pdf(BSDFQueryRecord& bRec) const;

    bool isDiffuse() { return true; }

    std::string toString() const;

private:
    Color3f m_albedo;
};

LUMINA_NAMESPACE_END
