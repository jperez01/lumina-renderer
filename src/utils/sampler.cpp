//
// Created by juperez on 5/26/23.
//

#include "sampler.h"

LUMINA_NAMESPACE_BEGIN

Independent::Independent(const PropertyList &propsList) {
    m_sampleCount = (size_t) propsList.getInteger("sampleCount", 1);
}

std::unique_ptr<Sampler> Independent::clone() const {
    std::unique_ptr<Independent> cloned(new Independent());
    cloned->m_sampleCount = m_sampleCount;
    cloned->m_random = m_random;

    return std::move(cloned);
}

void Independent::prepare(const ImageBlock &block) {
    m_random.seed(
            block.getOffset().x(),
            block.getOffset().y()
            );
}

void Independent::generate() { }

void Independent::advance() { }

float Independent::next1D() {
    return m_random.nextFloat();
}

Point2f Independent::next2D() {
    return Point2f(m_random.nextFloat(), m_random.nextFloat());
}
std::string Independent::toString() const {
    return tfm::format("Independent[sampleCount=%i]", m_sampleCount);
}

    LUMINA_REGISTER_CLASS(Independent, "independent")
LUMINA_NAMESPACE_END
