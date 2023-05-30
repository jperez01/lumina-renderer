//
// Created by juperez on 5/28/23.
//

#pragma once

#include "emitter.h"

LUMINA_NAMESPACE_BEGIN

class AreaLight : public Emitter {
public:
    AreaLight(const PropertyList &props);

    Color3f sample(EmitterQueryRecord &record, const Point2f &sample) const;

    float pdf() const;

    Color3f eval(const EmitterQueryRecord &record) const;

    void setParent(LuminaObject *parent);

    std::string toString() const;
    Color3f getRadiance() const { return m_radiance; }

private:
    Color3f m_radiance;
};

LUMINA_NAMESPACE_END