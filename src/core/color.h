//
// Created by juperez on 5/24/23.
//

#pragma once

#include "common.h"

struct Color3f : public Eigen::Array3f {
public:
    typedef Eigen::Array3f Base;

    Color3f(float value = 0.f) : Base(value, value, value) {}
    Color3f(float r, float g, float b) : Base(r, g, b) {}

    template<typename Derived> Color3f(const Eigen::ArrayBase<Derived>& p)
    : Base(p) {}

    template <typename Derived> Color3f &operator=(const Eigen::ArrayBase<Derived>& p) {
        this->Base::operator=(p);
        return *this;
    }

    /// Return a reference to the red channel
    float &r() { return x(); }
    /// Return a reference to the red channel (const version)
    const float &r() const { return x(); }
    /// Return a reference to the green channel
    float &g() { return y(); }
    /// Return a reference to the green channel (const version)
    const float &g() const { return y(); }
    /// Return a reference to the blue channel
    float &b() { return z(); }
    /// Return a reference to the blue channel (const version)
    const float &b() const { return z(); }

    Color3f clamp() const {
        return Color3f(std::max(r(), 0.0f), std::max(g(), 0.0f), std::max(b(), 0.0f));
    }

    bool isValid() const;

    Color3f toLinearRGB() const;
    Color3f toSRGB() const;

    float getLuminance() const;

    std::string toString() const {
        return tfm::format("[%f, %f, %f]", coeff(0), coeff(1), coeff(2));
    }
};

struct Color4f : public Eigen::Array4f {
public:
    typedef Eigen::Array4f Base;

    Color4f() : Base(0.0f, 0.0f, 0.0f, 0.0f) { }

    Color4f(const Color3f &c) : Base(c.r(), c.g(), c.b(), 1.0f) { }

    Color4f(float r, float g, float b, float w) : Base(r, g, b, w) { }

    template <typename Derived> Color4f(const Eigen::ArrayBase<Derived>& p)
            : Base(p) { }

    template <typename Derived> Color4f &operator=(const Eigen::ArrayBase<Derived>& p) {
        this->Base::operator=(p);
        return *this;
    }

    Color3f divideByFilterWeight() const {
        if (w() != 0)
            return head<3>() / w();
        else
            return Color3f(0.0f);
    }

    std::string toString() const {
        return tfm::format("[%f, %f, %f, %f]", coeff(0), coeff(1), coeff(2), coeff(3));
    }
};
