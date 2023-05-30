//
// Created by juperez on 5/28/23.
//

#include "integrator.h"
#include "utils/warp.h"

LUMINA_NAMESPACE_BEGIN

#define MAX_BOUNCES 5

class PathEMSIntegrator : public Integrator {
public:
    PathEMSIntegrator(const PropertyList& propsList) {}

    Color3f Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const {
        Color3f totalColor(0.0f), throughput(1.0f);
        int numBounces = 0;
        float rrProb = std::fmin(0.99, throughput.maxCoeff()), eta = 1.0f;
        Intersection its;
        Ray3f someRay(ray);
        bool haveEmitterColor = false;

        while (scene->rayIntersect(someRay, its) && numBounces < MAX_BOUNCES) {
            if (its.mesh->isEmitter() && !haveEmitterColor) {
                EmitterQueryRecord emitterRecord(its.p);
                emitterRecord.n = its.shadingFrame.n;
                emitterRecord.wi = someRay.d;

                totalColor += throughput * its.mesh->getEmitter()->eval(emitterRecord);
                haveEmitterColor = true;
            }

            BSDFQueryRecord record(its.toLocal(-someRay.d));
            Color3f bsdfColor = its.mesh->getBSDF()->sample(record, sampler->next2D());

            float lightPdf;
            Emitter* emitter = scene->sampleLight(sampler->next1D(), lightPdf);

            EmitterQueryRecord emitterRecord(its.p, its.shadingFrame.n);
            Color3f Le = emitter->sample(emitterRecord, sampler->next2D());

            Intersection shadowIts;
            Ray3f shadowRay(its.p + its.shadingFrame.n * 0.001f, emitterRecord.wi);
            if (!scene->rayIntersect(shadowRay, shadowIts)) {
                totalColor += Le * bsdfColor / (emitterRecord.pdf * lightPdf);
            }

            throughput *= bsdfColor / (rrProb);
            eta *= record.eta;

            if (numBounces > 3) {
                rrProb = std::fmin(0.99f, throughput.maxCoeff() * eta * eta);

                if (sampler->next1D() > rrProb) {
                    break;
                }
            }
            someRay = Ray3f(its.p, its.toWorld(record.wo));
            numBounces++;
        }

        return totalColor;
    }

    std::string toString() const {
        return "Path Integrator[]";
    }
};

LUMINA_REGISTER_CLASS(PathEMSIntegrator, "path_ems")
LUMINA_NAMESPACE_END