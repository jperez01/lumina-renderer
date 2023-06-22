#include "bsdf.h"
#include "primitives/frame.h"
#include "utils/warp.h"
#include "textures/texture.h"

LUMINA_NAMESPACE_BEGIN

class Microfacet : public BSDF {
public:
    Microfacet(const PropertyList& propList) {
        /* RMS surface roughness */
        m_alpha = propList.getFloat("alpha", 0.1f);

        /* Interior IOR (default: BK7 borosilicate optical glass) */
        m_intIOR = propList.getFloat("intIOR", 1.5046f);

        /* Exterior IOR (default: air) */
        m_extIOR = propList.getFloat("extIOR", 1.000277f);

        /* Albedo of the diffuse base material (a.k.a "kd") */
        m_kd = propList.getColor("kd", Color3f(0.5f));

        /* To ensure energy conservation, we must scale the
           specular component by 1-kd.

           While that is not a particularly realistic model of what
           happens in reality, this will greatly simplify the
           implementation. Please see the course staff if you're
           interested in implementing a more realistic version
           of this BRDF. */
        m_ks = 1 - m_kd.maxCoeff();
    }

    /// Evaluate the BRDF for the given pair of directions
    Color3f eval(const BSDFQueryRecord& bRec) const {
        if (Frame::cosTheta(bRec.wi) <= 0.0f || Frame::cosTheta(bRec.wo) <= 0.0f)
            return Color3f(0.0f);

        Color3f albedo = m_kd;
        if (albedoTexture) {
            Intersection its;
            its.uv = bRec.uv;
            albedo = albedoTexture.get()->evaluate(its);
        }

        float ks = m_ks, kd = 1 - ks;
        if (metallicTexture) {
            Intersection its;
            its.uv = bRec.uv;

            ks = metallicTexture.get()->evaluate(its);
            kd = 1.0f - ks;

            albedo = lerp(ks, Color3f(0.04), albedo);
        }

        Color3f diffusePart = albedo / M_PI;

        Vector3f sumVector = bRec.wi + bRec.wo;
        Vector3f wh = sumVector.normalized();

        float fCoefficient = fresnel(wh.dot(bRec.wi), m_extIOR, m_intIOR);

        Vector3f z_axis(0, 0, 1);
        float cosThetai = bRec.wi.dot(z_axis), cosThetao = bRec.wo.dot(z_axis),
            cosThetah = wh.dot(z_axis);

        float roughness = m_alpha;
        if (roughnessTexture) {
            Intersection its;
            its.uv = bRec.uv;
            roughness = roughnessTexture.get()->evaluate(its);
        }

        float Gih = GShadowTerm(bRec.wi, wh, roughness), Goh = GShadowTerm(bRec.wo, wh, roughness);

        float d = Warp::squareToBeckmannPdf(wh, roughness);

        Color3f specularPart = ks * (d * fCoefficient * Gih * Goh) / (4.0f * cosThetah * cosThetai * cosThetao);

        return diffusePart + specularPart;
    }

    float GShadowTerm(Vector3f wv, Vector3f wh, float roughness) const {
        float cosThetav = Frame::cosTheta(wv);
        float c = wv.dot(wh) / cosThetav;
        float x = c > 0 ? 1 : 0;

        float b = 1.0f / (roughness * tan(acos(cosThetav)));
        float complexArg = 1.0f;
        if (b < 1.6) {
            float b2 = b * b;
            complexArg = (3.535f * b + 2.181f * b2) / (1 + 2.276 * b + 2.577 * b2);
        }

        return x * complexArg;
    }

    /// Evaluate the sampling density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord& bRec) const {
        if (Frame::cosTheta(bRec.wi) <= 0.0f || Frame::cosTheta(bRec.wo) <= 0.0f)
            return 0.0f;

        Vector3f wh = (bRec.wi + bRec.wo).normalized();

        float roughness = m_alpha;
        if (roughnessTexture) {
            Intersection its;
            its.uv = bRec.uv;
            roughness = roughnessTexture.get()->evaluate(its);
        }

        float D = Warp::squareToBeckmannPdf(wh, roughness);
        float J = 1 / (4.0f * wh.dot(bRec.wo));

        float cosThetao = Frame::cosTheta(bRec.wo);

        return m_ks * D * J + (1 - m_ks) * cosThetao * INV_PI;
    }

    /// Sample the BRDF
    Color3f sample(BSDFQueryRecord& bRec, const Point2f& _sample) const {
        if (Frame::cosTheta(bRec.wi) <= 0.0f)
            return Color3f(0.0f);
        bRec.measure = ESolidAngle;

        float roughness = m_alpha;
        if (roughnessTexture) {
            Intersection its;
            its.uv = bRec.uv;
            roughness = roughnessTexture.get()->evaluate(its);
        }
        //Remap sample to [0, 1] distribution because sample.x() will not be in the range
        if (_sample.x() < m_ks) {
            Point2f sampleReuse(_sample.x() / m_ks, _sample.y());
            Normal3f n = Warp::squareToBeckmann(sampleReuse, roughness);

            bRec.wo = reflect(bRec.wi, n).normalized();
        }
        else {
            Point2f sampleReuse((_sample.x() - m_ks) / (1.0f - m_ks), _sample.y());
            bRec.wo = Warp::squareToCosineHemisphere(sampleReuse);
        }
        bRec.eta = m_extIOR / m_intIOR;

        if (Frame::cosTheta(bRec.wo) < 0.0f)
            return Color3f(0.0f);

        return eval(bRec) * Frame::cosTheta(bRec.wo) / pdf(bRec);

        // Note: Once you have implemented the part that computes the scattered
        // direction, the last part of this function should simply return the
        // BRDF value divided by the solid angle density and multiplied by the
        // cosine factor from the reflection equation, i.e.
        // return eval(bRec) * Frame::cosTheta(bRec.wo) / pdf(bRec);
    }

    bool isDiffuse() const {
        /* While microfacet BRDFs are not perfectly diffuse, they can be
           handled by sampling techniques for diffuse/non-specular materials,
           hence we return true here */
        return true;
    }

    void addChild(LuminaObject* child) {
        if (child->getTemplatedClassType() == EColorTexture) {
            Texture<Color3f>* texture = dynamic_cast<Texture<Color3f>*>(child);

            switch (texture->getTextureType()) {
            case Albedo:
                albedoTexture.reset(texture);
                break;
            
            default:
                break;
            }
        }
        else if (child->getTemplatedClassType() == EFloatTexture) {
            Texture<float>* texture = dynamic_cast<Texture<float>*>(child);

            switch (texture->getTextureType()) {
            case Metallic:
                metallicTexture.reset(texture);
                break;
            case Roughness:
                roughnessTexture.reset(texture);
                break;
            default:
                break;
            }
        }
        else {
            throw LuminaException("Texture type not allowed");
        }
    }

    std::string toString() const {
        return tfm::format(
            "Microfacet[\n"
            "  alpha = %f,\n"
            "  intIOR = %f,\n"
            "  extIOR = %f,\n"
            "  kd = %s,\n"
            "  ks = %f\n"
            "]",
            m_alpha,
            m_intIOR,
            m_extIOR,
            m_kd.toString(),
            m_ks
        );
    }
private:
    float m_alpha;
    float m_intIOR, m_extIOR;
    float m_ks;
    Color3f m_kd;

    std::unique_ptr<Texture<Color3f>> albedoTexture;
    std::unique_ptr<Texture<float>> metallicTexture;
    std::unique_ptr<Texture<float>> roughnessTexture;
};

LUMINA_REGISTER_CLASS(Microfacet, "microfacet")
LUMINA_NAMESPACE_END