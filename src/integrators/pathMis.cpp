//
// Created by juperez on 5/28/23.
//

#include "integrator.h"

LUMINA_NAMESPACE_BEGIN

#define MAX_BOUNCES 6

class PathMisIntegrator : public Integrator {
public:
    PathMisIntegrator(const PropertyList& propsList) {}

    Color3f Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const {
        Color3f totalColor(0.0f), throughput(1.0f);
        int numBounces = 0;
        float rrProb = std::fmin(0.99, throughput.maxCoeff()), eta = 1.0f;
        Intersection its;
        Ray3f someRay(ray);
        bool foundLight = false;

        while (scene->rayIntersect(someRay, its) && numBounces < MAX_BOUNCES) {
            if (its.mesh->isEmitter() && !foundLight) {
                EmitterQueryRecord emitterRecord(its.p);
                emitterRecord.n = its.geoFrame.n;
                //emitterRecord.n = its.shadingFrame.n;
                emitterRecord.wi = someRay.d;
                Color3f emitterColor = its.mesh->getEmitter()->eval(emitterRecord);

                totalColor += throughput * emitterColor;
                foundLight = true;
            }

            totalColor += throughput * estimateDirect(scene, sampler, its, someRay);

            BSDFQueryRecord bsdfRecord(its.toLocal(-someRay.d));
            Color3f bsdfColor = its.mesh->getBSDF()->sample(bsdfRecord, sampler->next2D());

            throughput *= bsdfColor / rrProb;
            eta *= bsdfRecord.eta;

            if (numBounces > 3) {
                rrProb = std::fmin(0.99f, throughput.maxCoeff() * eta * eta);
                if (sampler->next1D() > rrProb)
                    break;
            }

            someRay = Ray3f(its.p, its.toWorld(bsdfRecord.wo));
            numBounces++;
        }

        return totalColor;
    }

    Color3f estimateDirect(const Scene* scene, Sampler* sampler, Intersection& its, Ray3f& ray) const {
        Color3f finalColor(0.0f);
        const BSDF* bsdf = its.mesh->getBSDF();

        // Sample from light source
        float lightPdf;
        Emitter* emitter = scene->sampleLight(sampler->next1D(), lightPdf);

        EmitterQueryRecord emitterRecord(its.p, its.shadingFrame.n);
        Color3f emitterColor = emitter->sample(emitterRecord, sampler->next2D());
        convertToSolidAngle(emitterRecord);

        if (emitterRecord.pdf > 0.0f) {
            BSDFQueryRecord hypotheticalBsdfRecord(its.toLocal(emitterRecord.wi), 
                its.toLocal(-ray.d), ESolidAngle);
            float hypoBsdfPdf = bsdf->pdf(hypotheticalBsdfRecord);
            Color3f hypoBsdfColor = bsdf->eval(hypotheticalBsdfRecord)
                * std::max(0.0f, hypotheticalBsdfRecord.wi.dot(its.shadingFrame.n));

            Ray3f shadowRay(its.p, emitterRecord.wi);
            Intersection shadowIts;
            bool hitObject = scene->rayIntersect(shadowRay, shadowIts);
            if (!hitObject || shadowIts.mesh->getEmitter() == emitter) {
                float emitterWeight = emitterRecord.pdf / (emitterRecord.pdf + hypoBsdfPdf);

                finalColor += emitterColor * hypoBsdfColor * emitterWeight / lightPdf;
            }
        }

        // Sample from mesh
        BSDFQueryRecord bsdfRecord(its.toLocal(-ray.d));
        Color3f bsdfColor = bsdf->sample(bsdfRecord, sampler->next2D());
        float bsdfPdf = bsdf->pdf(bsdfRecord);

        if (bsdfPdf > 0.0f) {
            Ray3f shadowRay = Ray3f(its.p, its.toWorld(bsdfRecord.wi));
            Intersection newShadowIts;
            bool hitObject = scene->rayIntersect(shadowRay, newShadowIts);

            if (hitObject && newShadowIts.mesh->isEmitter()) {
                Emitter* foundEmitter = newShadowIts.mesh->getEmitter();

                EmitterQueryRecord fakeEmitterRecord(shadowRay.o);
                fakeEmitterRecord.wi = its.toWorld(bsdfRecord.wi);
                fakeEmitterRecord.n = newShadowIts.geoFrame.n;
                fakeEmitterRecord.p = newShadowIts.p;
                fakeEmitterRecord.pdf = foundEmitter->pdf();

                convertToSolidAngle(fakeEmitterRecord);
                float emitterPdf = fakeEmitterRecord.pdf;
                Color3f emitterColor = foundEmitter->eval(fakeEmitterRecord);

                float bsdfSampleWeight = bsdfPdf / (bsdfPdf + emitterPdf);
                finalColor += bsdfColor * emitterColor * bsdfSampleWeight;
            }
        }

        return finalColor;
    }

    std::string toString() const {
        return "Path Mis Integrator[]";
    }
};

LUMINA_REGISTER_CLASS(PathMisIntegrator, "path_mis")
LUMINA_NAMESPACE_END