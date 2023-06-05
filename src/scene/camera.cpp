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

    m_filter = nullptr;
}

    void PerspectiveCamera::activate() {
        float aspect = m_outputSize.x() / (float) m_outputSize.y();

        /* Project vectors in camera space onto a plane at z=1:
         *
         *  xProj = cot * x / z
         *  yProj = cot * y / z
         *  zProj = (far * (z - near)) / (z * (far-near))
         *  The cotangent factor ensures that the field of view is
         *  mapped to the interval [-1, 1].
         */
        float recip = 1.0f / (m_farClip - m_nearClip),
                cot = 1.0f / std::tan(degToRad(m_fov / 2.0f));

        Eigen::Matrix4f perspective;
        perspective <<
                    cot, 0,   0,   0,
                0, cot,   0,   0,
                0,   0,   m_farClip * recip, -m_nearClip * m_farClip * recip,
                0,   0,   1,   0;

        /**
         * Translation and scaling to shift the clip coordinates into the
         * range from zero to one. Also takes the aspect ratio into account.
         */
        m_sampleToCamera = Transform(
                Eigen::DiagonalMatrix<float, 3>(Vector3f(-0.5f, -0.5f * aspect, 1.0f)) *
                Eigen::Translation<float, 3>(-1.0f, -1.0f/aspect, 0.0f) * perspective).inverse();

        /* If no reconstruction filter was assigned, instantiate a Gaussian filter */
        if (!m_filter)
            m_filter = static_cast<ReconstructionFilter *>(
                    LuminaObjectFactory::createInstance("gaussian", PropertyList()));
    }

    Color3f PerspectiveCamera::sampleRay(Ray3f &ray,
                      const Point2f &samplePosition,
                      const Point2f &apertureSample) const {
        /* Compute the corresponding position on the
           near plane (in local camera space) */
        Point3f nearP = m_sampleToCamera * Point3f(
                samplePosition.x() * m_invOutputSize.x(),
                samplePosition.y() * m_invOutputSize.y(), 0.0f);

        /* Turn into a normalized ray direction, and
           adjust the ray interval accordingly */
        Vector3f d = nearP.normalized();
        float invZ = 1.0f / d.z();

        ray.o = m_cameraToWorld * Point3f(0, 0, 0);
        ray.d = (m_cameraToWorld * d).normalized();
        ray.mint = m_nearClip * invZ;
        ray.maxt = m_farClip * invZ;
        ray.update();

        return Color3f(1.0f);
    }

void PerspectiveCamera::addChild(LuminaObject *obj) {
    if (obj->getClassType() == EReconstructionFilter) {
        if (m_filter)
            throw LuminaException("Filter is already defined.");

        m_filter = dynamic_cast<ReconstructionFilter*>(obj);
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
            indent(m_filter->toString())
    );
}

LUMINA_REGISTER_CLASS(PerspectiveCamera, "perspective")
LUMINA_NAMESPACE_END
