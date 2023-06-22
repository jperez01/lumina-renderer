//
// Created by juperez on 5/28/23.
//

#include "areaLight.h"

LUMINA_NAMESPACE_BEGIN

AreaLight::AreaLight(const PropertyList &props) {
    m_radiance = props.getColor("radiance", Color3f(0.5f));
}

Color3f AreaLight::sample(EmitterQueryRecord &record, const Point2f &sample) const {
    m_mesh->samplePosition(sample, record.p, record.n);

    record.oToP = (record.p - record.refOrigin);

    record.wi = record.oToP.normalized();
    record.pdf = pdf();

    float distance = record.oToP.dot(record.oToP);
    float numerator = abs(record.refNormal.dot(record.wi)) * abs(record.n.dot(-record.wi));

    if (distance == 0.0f || record.pdf == 0.0f) {
        record.pdf = 0.0f;
        return Color3f(0.0f);
    }

    return eval(record) * numerator / distance;
}

float AreaLight::pdf() const {
    /*
    * // Convert light sample weight to solid angle measure
    Float pdf = DistanceSquared(ref.p, isectLight.p) /
                (AbsDot(isectLight.n, -wi) * Area());
    */
    return m_mesh->pdf();
}

Color3f AreaLight::eval(const EmitterQueryRecord &record) const {
    float cosTheta = record.n.dot(-record.wi);
    if (cosTheta >= 0.0f)
        return m_radiance;

    return Color3f(0.0f);
}

void AreaLight::setParent(LuminaObject *parent) {
    switch (parent->getClassType()) {
        case EMesh: {
                m_mesh = dynamic_cast<Mesh*>(parent);
            }
            break;
        default:
            throw LuminaException("Class type %s not supported by AreaLight!", parent->getClassType());
    }
}

std::string AreaLight::toString() const {
    return tfm::format(
            "AreaLight[\n %s \n]",
            m_radiance
            );
}

LUMINA_REGISTER_CLASS(AreaLight, "area")
LUMINA_NAMESPACE_END