//
// Created by juperez on 5/25/23.
//

#pragma once

#include "core/object.h"

LUMINA_NAMESPACE_BEGIN

struct BSDFQueryRecord {
    Vector3f wi;
    Vector3f wo;

    float eta;

    // Measure associated with the sample
    EMeasure measure;

    BSDFQueryRecord(const Vector3f& wi) : wi(wi), eta(1.0f), measure(EUnknownMeasure) {}

    BSDFQueryRecord(const Vector3f& wi, const Vector3f& wo, EMeasure measure)
    : wi(wi), wo(wo), eta(1.0f), measure(measure) {}
};

class BSDF : public LuminaObject {
public:
    virtual Color3f sample(BSDFQueryRecord& bRec, const Point2f& sample) const = 0;

    virtual Color3f eval(BSDFQueryRecord& bRec) const = 0;

    virtual float pdf(BSDFQueryRecord& bRec) const = 0;

    EClassType getClassType() const { return EBSDF; }

    virtual bool isDiffuse() { return false; }
};

LUMINA_NAMESPACE_END