#include "emitter.h"

LUMINA_NAMESPACE_BEGIN

class PointLight : public Emitter {
public:
    PointLight(const PropertyList& props) {
        m_radiance = props.getColor("radiance", Color3f(0.5f));
        m_position = props.getPoint("position", Point3f(0.0f));
    }

    Color3f sample(EmitterQueryRecord& record, const Point2f& sample) const {
        record.oToP = m_position - record.refOrigin;
        record.wi = record.oToP.normalized();
        record.pdf = 1.0f;

        float distance = record.oToP.dot(record.oToP);
        float numerator = abs(record.refNormal.dot(record.wi));

        return eval(record) * numerator / (distance * distance);
    }

    float pdf() const {
        return 1.0f;
    }

    Color3f eval(const EmitterQueryRecord& record) const {
        return m_radiance;
    }

    std::string toString() const {
        return tfm::format(
            "PointLight[\n Radiance: %s  \n Position: %s \n]",
            m_radiance,
            m_position
        );
    }
    Color3f getRadiance() const { return m_radiance; }

private:
    Color3f m_radiance;
    Point3f m_position;
};

LUMINA_REGISTER_CLASS(PointLight, "pointLight")
LUMINA_NAMESPACE_END