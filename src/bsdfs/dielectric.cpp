//
// Created by juperez on 5/28/23.
//

#include "bsdf.h"
#include "primitives/frame.h"

LUMINA_NAMESPACE_BEGIN

/// Ideal dielectric BSDF
class Dielectric : public BSDF {
public:
    Dielectric(const PropertyList &propList) {
        /* Interior IOR (default: BK7 borosilicate optical glass) */
        m_intIOR = propList.getFloat("intIOR", 1.5046f);

        /* Exterior IOR (default: air) */
        m_extIOR = propList.getFloat("extIOR", 1.000277f);
    }

    Color3f eval(const BSDFQueryRecord& bRec) const {
        /* Discrete BRDFs always evaluate to zero in Nori */
        return Color3f(0.0f);
    }

    float pdf(const BSDFQueryRecord& bRec) const {
        /* Discrete BRDFs always evaluate to zero in Nori */
        return 0.0f;
    }

    Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
        float cosTheta = Frame::cosTheta(bRec.wi);

        float Fr = fresnel(cosTheta, m_extIOR, m_intIOR);
        bRec.measure = EDiscrete;

        if (sample.x() <= Fr) {
            bRec.wo = Vector3f(
                    -bRec.wi.x(),
                    -bRec.wi.y(),
                    bRec.wi.z()
            );
            bRec.eta = 1.0f;
        } else {
            float eta = cosTheta < 0 ? m_intIOR / m_extIOR : m_extIOR / m_intIOR;
            Normal3f n = cosTheta < 0 ? Normal3f(0.0f, 0.0f, -1.0f) : Normal3f(0.0f, 0.0f, 1.0f);

            Vector3f term1 = -eta * (bRec.wi - std::abs(cosTheta) * n);
            Vector3f term2 = -std::sqrt(1.0f - eta * eta * (1.0 - cosTheta * cosTheta)) * n;

            bRec.wo = term1 + term2;
            bRec.wo.normalize();
            bRec.eta = eta;
        }

        return Color3f(1.0f / (bRec.eta * bRec.eta));
    }

    Color3f getRadiance() const { return Color3f(0.0f); }

    std::string toString() const {
        return tfm::format(
                "Dielectric[\n"
                "  intIOR = %f,\n"
                "  extIOR = %f\n"
                "]",
                m_intIOR, m_extIOR);
    }
private:
    float m_intIOR, m_extIOR;
};

LUMINA_REGISTER_CLASS(Dielectric, "dielectric");
LUMINA_NAMESPACE_END