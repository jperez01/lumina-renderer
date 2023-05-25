//
// Created by juperez on 5/24/23.
//

#pragma once
#include "core/object.h"

LUMINA_NAMESPACE_BEGIN

class Camera : public LuminaObject {
public:
    const Vector2i &getOutputSize() const { return m_outputSize; }
    EClassType getClassType() const { return ECamera; }

protected:
    Vector2i m_outputSize;
};

class PerspectiveCamera : public Camera {
public:
    PerspectiveCamera(const PropertyList& propsList);

    void activate();
    Color3f sampleRay(Ray3f &ray, const Point2f& samplePosition, const Point2f& apertureSample) const;
    void addChild(LuminaObject* obj);

    std::string toString() const;
private:
    Vector2f m_invOutputSize;
    Transform m_sampleToCamera;
    Transform m_cameraToWorld;

    float m_fov, m_nearClip, m_farClip;
};

LUMINA_NAMESPACE_END