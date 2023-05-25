//
// Created by juperez on 5/24/23.
//

#include "camera.h"
#include <Eigen/Geometry>

LUMINA_NAMESPACE_BEGIN

PerspectiveCamera::PerspectiveCamera(const PropertyList& propsList) {
    m_outputSize.x() = propsList.getInteger("width", 1280);
    m_outputSize.y() = propsList.getInteger("height", 720);
    m_invOutputSize = m_outputSize.cast<float>().cwiseInverse();

    m_cameraToWorld = propsList.getTransform("toWorld", Transform());

    m_fov = propsList.getFloat("fov", 30.0f);
    m_nearClip = propsList.getFloat("nearClip", 1e-4f);
    m_farClip = propsList.getFloat("farClip", 1e4f);
}

void PerspectiveCamera::activate() {
    float aspect = m_outputSize.x() / (float) m_outputSize.y();
    float denom = std::tan(degToRad(m_fov / 2.0f));
    float scale = 1.0f / denom;

    float recip = 1.0f / (m_farClip - m_nearClip);

    Eigen::Matrix4f matrix;

    matrix <<
        scale, 0, 0, 0,
        0, scale, 0, 0,
        0, 0, m_farClip * recip, -(m_farClip * m_nearClip) * recip,
        0, 0, 1, 0;

    m_sampleToCamera = Transform(
            Eigen::DiagonalMatrix<float, 3>(Vector3f(-0.5f)) *
                    Eigen::Translation<float, 3>(-1.0f, -1.0f / aspect, 0.0f) * matrix).inverse();
}

Color3f
PerspectiveCamera::sampleRay(Ray3f &ray, const Point2f &samplePosition, const Point2f &apertureSample) const {
    Point3f nearPos = m_sampleToCamera * Point3f(
            samplePosition.x() * m_invOutputSize.x(),
            samplePosition.y() * m_invOutputSize.y(), 0.0f
            );

    Vector3f dir = nearPos.normalized();
    float invZ = 1.0f / dir.z();

    ray.o = m_cameraToWorld * Point3f(0.0f);
    ray.d = m_cameraToWorld * dir;
    ray.mint = m_nearClip  * invZ;
    ray.maxt = m_farClip * invZ;
    ray.update();

    return Color3f(1.0f);
}

void PerspectiveCamera::addChild(LuminaObject *obj) {
    if (obj->getClassType() == EReconstructionFilter) {

    } else {
        throw LuminaException("Camera::addChild(<%s>) is not supported!",
                              classTypeName(obj->getClassType()));
    }
}

std::string PerspectiveCamera::toString() const {
    return tfm::format(
            "Perspective Camera[\n"
            "  cameraToWorld = %s, \n"
            "  outputSize = %s,\n"
            "  fov = %f,\n"
            "  clip = [%f, %f],\n"
            "  rfilter = %s\n"
            "]",
            indent(m_cameraToWorld.toString(), 18),
            m_outputSize.toString(),
            m_fov,
            m_nearClip, m_farClip,
            "None"
    );
}

LUMINA_REGISTER_CLASS(PerspectiveCamera, "perspective")
LUMINA_NAMESPACE_END
