//
// Created by juperez on 5/26/23.
//

#include "color.h"

Color3f Color3f::toSRGB() const {
    Color3f result;

    for (int i=0; i<3; ++i) {
        float value = coeff(i);

        if (value <= 0.0031308f)
            result[i] = 12.92f * value;
        else
            result[i] = (1.0f + 0.055f)
                        * std::pow(value, 1.0f/2.4f) -  0.055f;
    }

    return result;
}

Color3f Color3f::toLinearRGB() const {
    Color3f result;

    for (int i=0; i<3; ++i) {
        float value = coeff(i);

        if (value <= 0.04045f)
            result[i] = value * (1.0f / 12.92f);
        else
            result[i] = std::pow((value + 0.055f)
                                 * (1.0f / 1.055f), 2.4f);
    }

    return result;
}

bool Color3f::isValid() const {
    for (int i=0; i<3; ++i) {
        float value = coeff(i);
        if (value < 0 || !std::isfinite(value))
            return false;
    }
    return true;
}

float Color3f::getLuminance() const {
    return coeff(0) * 0.212671f + coeff(1) * 0.715160f + coeff(2) * 0.072169f;
}