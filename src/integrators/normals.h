//
// Created by juperez on 5/26/23.
//

#pragma once

#include "integrator.h"

LUMINA_NAMESPACE_BEGIN

class NormalIntegrator : public Integrator {
public:
    NormalIntegrator(const PropertyList& propsList);

    Color3f Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const;

    std::string toString() const;
};

LUMINA_NAMESPACE_END
