//
// Created by juperez on 5/26/23.
//

#pragma once

#include "core/object.h"
#include "pcg32/pcg32.h"
#include "image/block.h"

LUMINA_NAMESPACE_BEGIN

class Sampler : public LuminaObject {
public:
    virtual ~Sampler() {}

    virtual std::unique_ptr<Sampler> clone() const = 0;

    virtual void prepare(const ImageBlock& block) = 0;

    virtual void generate() = 0;
    virtual void advance() = 0;

    virtual float next1D() = 0;
    virtual Point2f next2D() = 0;

    void addChild(lumina::LuminaObject *child) override {

    }

    virtual size_t getSampleCount() const { return m_sampleCount; }
    EClassType getClassType() const { return ESampler; }

protected:
    size_t m_sampleCount;
};

class Independent : public Sampler {
public:
    Independent(const PropertyList& propsList);

    virtual ~Independent() {}

    std::unique_ptr<Sampler> clone() const;

    void prepare(const ImageBlock& block);
    void generate();
    void advance();

    float next1D();
    Point2f next2D();

    std::string toString() const;
protected:
    Independent() {}

private:
    pcg32 m_random;
};

LUMINA_NAMESPACE_END