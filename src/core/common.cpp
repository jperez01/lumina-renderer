//
// Created by juperez on 5/23/23.
//

#include "common.h"

#include <Eigen/Geometry>
#include <Eigen/LU>
#include "object.h"

LUMINA_NAMESPACE_BEGIN

std::vector<std::string> tokenize(const std::string &s, const std::string &delim, bool includeEmpty) {
    std::vector<std::string> tokens;

    std::string::size_type lastPos = 0, position = s.find_first_of(delim, lastPos);
    while (lastPos != std::string::npos) {
        if (position != lastPos || includeEmpty) {
            tokens.push_back(s.substr(lastPos, position - lastPos));
        }
        lastPos = position;
        if (lastPos != std::string::npos) {
            lastPos += 1;
            position = s.find_first_of(delim, lastPos);
        }
    }

    return tokens;
}

Eigen::Vector3f toVector3f(const std::string &str) {
    std::vector<std::string> tokens = tokenize(str);
    if (tokens.size() != 3)
        std::cout << "Expected 3 values \n";

    Eigen::Vector3f result;
    for (int i = 0; i < 3; i++)
        result[i] = toFloat(tokens[i]);

    return result;
}

float toFloat(const std::string &str) {
    char* end_ptr = nullptr;
    float value = strtof(str.c_str(), &end_ptr);

    if (*end_ptr != '\0')
        std::cout << "Could not parse floating point from string \n";

    return value;
}

int toInt(const std::string &str) {
    char* end_ptr = nullptr;
    int value = (int) strtol(str.c_str(), &end_ptr, 10);

    if (*end_ptr != '\0')
        std::cout << "Could not parse floating point from string \n";

    return value;
}

unsigned int toUInt(const std::string &str) {
    char* end_ptr = nullptr;
    unsigned int value = (unsigned int) strtol(str.c_str(), &end_ptr, 10);

    if (*end_ptr != '\0')
        std::cout << "Could not parse floating point from string \n";

    return value;
}

bool toBool(const std::string &str) {
    if (str == "false")
        return false;
    else if (str == "true")
        return true;
    else
        std::cout << "Could not parse bool from string \n";

    return false;
}

std::string indent(const std::string &string, int amount) {
    /* This could probably be done faster (it's not
       really speed-critical though) */
    std::istringstream iss(string);
    std::ostringstream oss;
    std::string spacer(amount, ' ');
    bool firstLine = true;
    for (std::string line; std::getline(iss, line); ) {
        if (!firstLine)
            oss << spacer;
        oss << line;
        if (!iss.eof())
            oss << std::endl;
        firstLine = false;
    }
    return oss.str();
}

std::string timeString(double time, bool precise) {
    if (std::isnan(time) || std::isinf(time))
        return "inf";

    std::string suffix = "ms";
    if (time > 1000) {
        time /= 1000; suffix = "s";
        if (time > 60) {
            time /= 60; suffix = "m";
            if (time > 60) {
                time /= 60; suffix = "h";
                if (time > 12) {
                    time /= 12; suffix = "d";
                }
            }
        }
    }

    std::ostringstream os;
    os << std::setprecision(precise ? 4 : 1)
       << std::fixed << time << suffix;

    return os.str();
}

std::string memString(size_t size, bool precise) {
    double value = (double) size;
    const char *suffixes[] = {
            "B", "KiB", "MiB", "GiB", "TiB", "PiB"
    };
    int suffix = 0;
    while (suffix < 5 && value > 1024.0f) {
        value /= 1024.0f; ++suffix;
    }

    std::ostringstream os;
    os << std::setprecision(suffix == 0 ? 0 : (precise ? 4 : 1))
       << std::fixed << value << " " << suffixes[suffix];

    return os.str();
}

Resolver* getFileResolver() {
    static Resolver* resolver = new Resolver();

    return resolver;
}

std::string toLower(const std::string &value) {
    std::string result;
    result.resize(value.size());
    std::transform(value.begin(), value.end(), result.begin(), ::tolower);
    return result;
}

bool endsWith(const std::string &value, const std::string &ending) {
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void coordinateSystem(const Vector3f &a, Vector3f &b, Vector3f &c) {
    if (std::abs(a.x()) > std::abs(a.y())) {
        float invLen = 1.0f / std::sqrt(a.x() * a.x() + a.z() * a.z());
        c = Vector3f(a.z() * invLen, 0.0f, -a.x() * invLen);
    } else {
        float invLen = 1.0f / std::sqrt(a.y() * a.y() + a.z() * a.z());
        c = Vector3f(0.0f, a.z() * invLen, -a.y() * invLen);
    }
    b = c.cross(a);
}

Transform::Transform(const Eigen::Matrix4f &trafo)
        : m_transform(trafo), m_inverse(trafo.inverse()) { }

std::string Transform::toString() const {
    std::ostringstream oss;
    oss << m_transform.format(Eigen::IOFormat(4, 0, ", ", ";\n", "", "", "[", "]"));
    return oss.str();
}

Transform Transform::operator*(const Transform &t) const {
    return Transform(m_transform * t.m_transform,
                     t.m_inverse * m_inverse);
}

float fresnel(float cosThetaI, float extIOR, float intIOR) {
    float etaI = extIOR, etaT = intIOR;

    if (extIOR == intIOR)
        return 0.0f;

    /* Swap the indices of refraction if the interaction starts
       at the inside of the object */
    if (cosThetaI < 0.0f) {
        std::swap(etaI, etaT);
        cosThetaI = -cosThetaI;
    }

    /* Using Snell's law, calculate the squared sine of the
       angle between the normal and the transmitted ray */
    float eta = etaI / etaT,
            sinThetaTSqr = eta*eta * (1-cosThetaI*cosThetaI);

    if (sinThetaTSqr > 1.0f)
        return 1.0f;  /* Total internal reflection! */

    float cosThetaT = std::sqrt(1.0f - sinThetaTSqr);

    float Rs = (etaI * cosThetaI - etaT * cosThetaT)
               / (etaI * cosThetaI + etaT * cosThetaT);
    float Rp = (etaT * cosThetaI - etaI * cosThetaT)
               / (etaT * cosThetaI + etaI * cosThetaT);

    return (Rs * Rs + Rp * Rp) / 2.0f;
}

LUMINA_NAMESPACE_END