//
// Created by juperez on 5/26/23.
//

#pragma once

#include "core/object.h"
#include "scene/scene.h"

LUMINA_NAMESPACE_BEGIN

class Integrator : public LuminaObject {
public:
    virtual ~Integrator() {}

    virtual void preprocess(const Scene* scene) {}

    virtual Color3f Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const = 0;

    EClassType getClassType() const { return EIntegrator; }
};

LUMINA_NAMESPACE_END
