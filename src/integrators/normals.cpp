//
// Created by juperez on 5/26/23.
//

#include "normals.h"

LUMINA_NAMESPACE_BEGIN

NormalIntegrator::NormalIntegrator(const PropertyList& propsList) {}

Color3f NormalIntegrator::Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const {
    Intersection its;
    if (!scene->rayIntersect(ray, its))
        return Color3f(0.0f);

    Normal3f normal = its.shadingFrame.n.cwiseAbs();

    return Color3f(normal.x(), normal.y(), normal.z());
}

std::string NormalIntegrator::toString() const {
    return "NormalIntegrator[]";
}

LUMINA_REGISTER_CLASS(NormalIntegrator, "normals")
LUMINA_NAMESPACE_END