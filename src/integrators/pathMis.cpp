//
// Created by juperez on 5/28/23.
//

#include "integrator.h"

LUMINA_NAMESPACE_BEGIN

#define MAX_BOUNCES 8

class PathMisIntegrator : public Integrator {
public:
    PathMisIntegrator(const PropertyList& propsList) {}

    Color3f Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const {
        Color3f totalColor(0.0f), throughput(1.0f);
        int numBounces = 0;
        float rrProb = std::fmin(0.99, throughput.maxCoeff()), eta = 1.0f;
        Intersection its;
        Ray3f someRay(ray);

        while (scene->rayIntersect(someRay, its) && numBounces < MAX_BOUNCES) {
            if (its.mesh->isEmitter() && numBounces == 0) {
                EmitterQueryRecord emitterRecord(its.p);
                emitterRecord.n = its.shadingFrame.n;
                emitterRecord.wi = someRay.d;

                totalColor += throughput * its.mesh->getEmitter()->eval(emitterRecord);
            }
            const BSDF* bsdf = its.mesh->getBSDF();
            float lightPdf;
            Emitter* emitter = scene->sampleLight(sampler->next1D(), lightPdf);

            EmitterQueryRecord emitterRecord(its.p, its.shadingFrame.n);
            Color3f emitterColor = emitter->sample(emitterRecord, sampler->next2D());

            BSDFQueryRecord fakeBsdfRecord(its.toLocal(emitterRecord.wi), its.toLocal(-ray.d), ESolidAngle);
            float emitterSampleWeight = emitterRecord.pdf / (emitterRecord.pdf + bsdf->pdf(fakeBsdfRecord));
            Color3f fakeBsdfColor = bsdf->eval(fakeBsdfRecord);

            Ray3f shadowRay(its.p, emitterRecord.wi);
            Intersection shadowIts;
            bool foundShadowIntersection = scene->rayIntersect(shadowRay, shadowIts);
            if (!foundShadowIntersection || shadowIts.mesh->getEmitter() != emitter) {
                totalColor += emitterColor * fakeBsdfColor * emitterSampleWeight / lightPdf;
            }

            BSDFQueryRecord bsdfRecord(its.toLocal(-someRay.d));
            Color3f bsdfColor = bsdf->sample(bsdfRecord, sampler->next2D());
            float bsdfPdf = bsdf->pdf(bsdfRecord);

            shadowRay = Ray3f(its.p, its.toWorld(bsdfRecord.wo));
            Intersection indirectShadowIts;
            bool hitSomething = scene->rayIntersect(shadowRay, indirectShadowIts);
            if (hitSomething && indirectShadowIts.mesh->isEmitter()) {
                float bsdfSampleWeight = bsdfPdf / (bsdfPdf + indirectShadowIts.mesh->getEmitter()->pdf());
                totalColor += bsdfColor * bsdfSampleWeight;
            }

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
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        return sampleLight(scene, sampler, someRay, its) + sampleBSDF(scene, sampler, someRay);
    }

    Color3f sampleLight(const Scene* scene, Sampler* sampler, Ray3f& ray, Intersection& its) const {
            float lightPdf;
            Emitter* emitter = scene->sampleLight(sampler->next1D(), lightPdf);

            EmitterQueryRecord emitterRecord(its.p);
            Color3f Le = emitter->sample(emitterRecord, sampler->next2D());

            BSDFQueryRecord bsdfRecord(its.toLocal(emitterRecord.wi),
                                       its.toLocal(-ray.d), ESolidAngle);
            Color3f bsdfColor = its.mesh->getBSDF()->eval(bsdfRecord);
            float bsdfPdf = its.mesh->getBSDF()->pdf(bsdfRecord);

            float weight = lightPdf / (lightPdf + bsdfPdf);

            Color3f directColor(0.0f);
            Ray3f shadowRay(its.p, emitterRecord.wi);
            Intersection shadowIts;
            bool foundShadowIntersection = scene->rayIntersect(shadowRay, shadowIts);
            if (!foundShadowIntersection || shadowIts.mesh->getEmitter() != emitter) {
                return Le * bsdfColor * weight / lightPdf;
            }
            return Color3f(0.0f);
    }

    Color3f sampleBSDF(const Scene* scene, Sampler* sampler, Ray3f& ray) const {
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
        return "Path Mis Integrator[]";
    }
};

LUMINA_REGISTER_CLASS(PathMisIntegrator, "path_mis")
LUMINA_NAMESPACE_END