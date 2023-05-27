//
// Created by juperez on 5/26/23.
//

#include "rfilter.h"

LUMINA_NAMESPACE_BEGIN
/**
 * Windowed Gaussian filter with configurable extent
 * and standard deviation. Often produces pleasing
 * results, but may introduce too much blurring.
 */
class GaussianFilter : public ReconstructionFilter {
public:
    GaussianFilter(const PropertyList &propList) {
        /* Half filter size */
        m_radius = propList.getFloat("radius", 2.0f);
        /* Standard deviation of the Gaussian */
        m_stddev = propList.getFloat("stddev", 0.5f);
    }

    float eval(float x) const {
        float alpha = -1.0f / (2.0f * m_stddev*m_stddev);
        return std::max(0.0f,
                        std::exp(alpha * x * x) -
                        std::exp(alpha * m_radius * m_radius));
    }

    std::string toString() const {
        return tfm::format("GaussianFilter[radius=%f, stddev=%f]", m_radius, m_stddev);
    }
protected:
    float m_stddev;
};

LUMINA_REGISTER_CLASS(GaussianFilter, "gaussian")
LUMINA_NAMESPACE_END