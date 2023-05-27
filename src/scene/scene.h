//
// Created by juperez on 5/24/23.
//

#pragma once

#include "core/object.h"
#include "camera.h"
#include "primitives/emitter.h"
#include "accel.h"
#include "utils/sampler.h"
#include "integrators/integrator.h"

LUMINA_NAMESPACE_BEGIN

class Scene : public LuminaObject {
public:
    /// Construct a new scene object
    Scene(const PropertyList &);

    /// Release all memory
    virtual ~Scene();

    /// Return a pointer to the scene's kd-tree
    const Accel *getAccel() const { return m_accel; }

    /// Return a pointer to the scene's integrator
    const Integrator *getIntegrator() const { return m_integrator; }

    /// Return a pointer to the scene's integrator
    Integrator *getIntegrator() { return m_integrator; }

    /// Return a pointer to the scene's camera
    const Camera *getCamera() const { return m_camera; }

    /// Return a pointer to the scene's sample generator (const version)
    const Sampler *getSampler() const { return m_sampler; }

    /// Return a pointer to the scene's sample generator
    Sampler *getSampler() { return m_sampler; }

    /// Return a reference to an array containing all meshes
    const std::vector<Mesh *> &getMeshes() const { return m_meshes; }
    const std::vector<Emitter *> &getLights() const { return m_emitters; }

    bool rayIntersect(const Ray3f& ray, Intersection& its) const;
    bool rayIntersect(const Ray3f& ray) const;

    /// \brief Return an axis-aligned box that bounds the scene
    const BoundingBox3f &getBoundingBox() const {
        return m_accel->getBoundingBox();
    }

    void activate();

    /// Add a child object to the scene (meshes, integrators etc.)
    void addChild(LuminaObject *obj);

    /// Return a string summary of the scene (for debugging purposes)
    std::string toString() const;

    EClassType getClassType() const { return EScene; }
private:
    Sampler* m_sampler = nullptr;
    Camera* m_camera = nullptr;
    Integrator* m_integrator = nullptr;
    Accel* m_accel = nullptr;

    std::vector<Mesh *> m_meshes;
    std::vector<Emitter *> m_emitters;
};

LUMINA_NAMESPACE_END
