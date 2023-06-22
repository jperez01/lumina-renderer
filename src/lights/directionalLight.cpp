#include "emitter.h"

LUMINA_NAMESPACE_BEGIN

class DirectionalLight : public Emitter {
public:
    DirectionalLight(const PropertyList& props) {
        m_radiance = props.getColor("radiance", Color3f(0.5f));
        m_direction = props.getVector("direction", Vector3f(0.0f, 1.0f, 0.0f));
        m_direction.normalize();
    }

    Color3f sample(EmitterQueryRecord& record, const Point2f& sample) const {
        record.wi = -m_direction;
        record.pdf = 1.0f;

        float numerator = abs(record.refNormal.dot(record.wi));

        return eval(record);
    }

    float pdf() const {
        return 1.0f;
    }

    Color3f eval(const EmitterQueryRecord& record) const {
        return m_radiance;
    }

    std::string toString() const {
        return tfm::format(
            "Directional Light[\n Radiance: %s  \n Direction: %s \n]",
            m_radiance,
            m_direction
        );
    }
    Color3f getRadiance() const { return m_radiance; }

private:
    Color3f m_radiance;
    Vector3f m_direction;
};

LUMINA_REGISTER_CLASS(DirectionalLight, "directionalLight")
LUMINA_NAMESPACE_END