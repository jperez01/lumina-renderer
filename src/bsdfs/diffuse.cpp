//
// Created by juperez on 5/25/23.
//

#include "diffuse.h"
#include "primitives/frame.h"
#include "utils/warp.h"

LUMINA_NAMESPACE_BEGIN

Diffuse::Diffuse(const PropertyList &propsList) {
    m_albedo = propsList.getColor("albedo", Color3f(0.5f));
}

Color3f Diffuse::sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
    if (Frame::cosTheta(bRec.wi) <= 0.0f) return Color3f(0.0f);

    bRec.measure = ESolidAngle;

    bRec.wo = Warp::squareToCosineHemisphere(sample);
    bRec.eta = 1.0f;

    return m_albedo;
}

Color3f Diffuse::eval(BSDFQueryRecord &bRec) const {
    if (bRec.measure != ESolidAngle
        || Frame::cosTheta(bRec.wi) <= 0.0f
        || Frame::cosTheta(bRec.wo) <= 0.0f)
        return Color3f(0.0f);

    return m_albedo * INV_PI;
}

float Diffuse::pdf(BSDFQueryRecord &bRec) const {
    if (bRec.measure != ESolidAngle
        || Frame::cosTheta(bRec.wi) <= 0.0f
        || Frame::cosTheta(bRec.wo) <= 0.0f)
        return 0.0f;

    return INV_PI * Frame::cosTheta(bRec.wo);
}

std::string Diffuse::toString() const {
    return tfm::format(
            "Diffuse[ albedo = %s] \n", m_albedo.toString()
            );
}

    LUMINA_REGISTER_CLASS(Diffuse, "diffuse")
LUMINA_NAMESPACE_END