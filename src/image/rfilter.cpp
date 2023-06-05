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

class BoxFilter : public ReconstructionFilter {
public:
    BoxFilter(const PropertyList& propList) {
        m_radius = propList.getFloat("radius", 2.0f);
    }

    float eval(float x) const {
        return 1.0f;
    }

    std::string toString() const {
        return tfm::format("BoxFilter[radius=%f]", m_radius);
    }
};

class TriangleFilter : public ReconstructionFilter {
public:
    TriangleFilter(const PropertyList& propList) {
        m_radius = propList.getFloat("radius", 2.0f);
    }

    float eval(float x) const {
        return std::max(0.0f, m_radius - std::abs(x));
    }

    std::string toString() const {
        return tfm::format("TriangleFilter[radius=%f]", m_radius);
    }
};

class LanczosSincFilter : public ReconstructionFilter {
public:
    LanczosSincFilter(const PropertyList& propList) {
        m_radius = propList.getFloat("radius", 2.0f);

        m_tau = propList.getFloat("tau", 3.0f);
    }

    float eval(float x) const {
        return windowedSinc(x, m_radius);
    }

    float sinc(float x) const {
        x = std::abs(x);
        if (x < Epsilon) return 1.0f;
        
        return std::sin(M_PI * x) / (M_PI * x);
    }

    float windowedSinc(float x, float radius) const {
        x = std::abs(x);
        if (x > radius) return 0.0f;

        float lanczos = sinc(x / m_tau);
        return sinc(x) * lanczos;
    }

    std::string toString() const {
        return tfm::format("LanczosSincFilter[radius=%f]", m_radius);
    }
protected:
    float m_tau;
};

class MitchellFilter : public ReconstructionFilter {
public:
    MitchellFilter(const PropertyList& propList) {
        m_radius = propList.getFloat("radius", 4.0f);
        B = propList.getFloat("B", 1.0f / 3.0f);
        C = propList.getFloat("C", 1.0f / 3.0f);
    }

    float eval(float x) const {
        return mitchell1d(x * m_radius);
    }

    float mitchell1d(float x) const {
        x = std::abs(2 * x);

        if (x > 1.0f)
            return ((-B - 6 * C) * x * x * x + (6 * B + 30 * C) * x * x +
                (-12 * B - 48 * C) * x + (8 * B + 24 * C)) *
            (1.f / 6.f);
        else
            return ((12 - 9 * B - 6 * C) * x * x * x +
                (-18 + 12 * B + 6 * C) * x * x + (6 - 2 * B)) *
            (1.f / 6.f);
    }

    std::string toString() const {
        return tfm::format("MitchellFilter[radius=%f]", m_radius);
    }
protected:
    float B, C;
};

LUMINA_REGISTER_CLASS(GaussianFilter, "gaussian")
LUMINA_REGISTER_CLASS(BoxFilter, "box")
LUMINA_REGISTER_CLASS(TriangleFilter, "triangle")
LUMINA_REGISTER_CLASS(LanczosSincFilter, "lanczos")
LUMINA_REGISTER_CLASS(MitchellFilter, "mitchell")
LUMINA_NAMESPACE_END