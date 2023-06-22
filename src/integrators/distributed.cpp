//
// Created by juperez on 5/29/23.
//

#include "integrator.h"

LUMINA_NAMESPACE_BEGIN

class DistributedIntegrator : public Integrator {
public:
    DistributedIntegrator(const PropertyList& propsList) {}

    Color3f Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const {
        Intersection its;
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        if (its.mesh->isEmitter()) {
            EmitterQueryRecord emitterRecord(its.p);
            emitterRecord.wi = ray.d;
            emitterRecord.n = its.shadingFrame.n;
            return its.mesh->getEmitter()->eval(emitterRecord);
        }

        const BSDF* bsdf = its.mesh->getBSDF();
        if (bsdf->isDiffuse()) {
            float lightPdf;
            Emitter* emitter = scene->sampleLight(sampler->next1D(), lightPdf);

            EmitterQueryRecord emitterRecord(its.p, its.shadingFrame.n);
            Color3f emitterColor = emitter->sample(emitterRecord, sampler->next2D());

            Color3f directColor(0.0f);
            Intersection shadowIts;
            Ray3f shadowRay(its.p, emitterRecord.wi);
            bool inShadow = scene->rayIntersect(shadowRay, shadowIts);
            if (!inShadow || shadowIts.mesh->getEmitter() == emitter) {
                BSDFQueryRecord bsdfRecord(its.toLocal(emitterRecord.wi), its.toLocal(-ray.d), ESolidAngle);
                bsdfRecord.uv = its.uv;
                Color3f albedo = its.mesh->getBSDF()->eval(bsdfRecord);

                directColor = albedo * emitterColor / (emitterRecord.pdf * lightPdf);
            }

            return directColor;
        } else {
            BSDFQueryRecord bsdfRecord(its.toLocal(-ray.d));
            bsdfRecord.uv = its.uv;
            Color3f bsdfColor = bsdf->sample(bsdfRecord, sampler->next2D());

            if (sampler->next1D() < 0.95f) {
                Ray3f newRay(its.p, its.toWorld(bsdfRecord.wo));
                return (1.0f/0.95f) * bsdfColor * Li(scene, sampler, newRay);
            } else
                return Color3f(0.0f);
        }
    }

    std::string toString() const {
        return "Whitted Integrator[]";
    }
};

LUMINA_REGISTER_CLASS(DistributedIntegrator, "whitted")
LUMINA_NAMESPACE_END