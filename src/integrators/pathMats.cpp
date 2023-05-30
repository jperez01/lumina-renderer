//
// Created by juperez on 5/29/23.
//
#include "integrator.h"
#include "utils/warp.h"

LUMINA_NAMESPACE_BEGIN

#define MAX_BOUNCES 8

    class PathMatsIntegrator : public Integrator {
    public:
        PathMatsIntegrator(const PropertyList& propsList) {}

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

    LUMINA_REGISTER_CLASS(PathMatsIntegrator, "path_mats")
LUMINA_NAMESPACE_END